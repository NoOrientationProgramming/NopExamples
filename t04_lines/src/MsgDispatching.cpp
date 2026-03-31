/*
  This file is part of the DSP-Crowd project
  https://www.dsp-crowd.com

  Author(s):
      - Johannes Natter, office@dsp-crowd.com

  File created on 31.03.2026

  Copyright (C) 2026, Johannes Natter

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#if defined(_WIN32)
#include <windows.h>
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#endif

#include <stdio.h>
#include <algorithm>
#include <sstream>
#include "MsgDispatching.h"

#include "env.h"

#define dForEach_ProcState(gen) \
		gen(StStart) \
		gen(StFileSearchWait) \
		gen(StMetricsWait) \
		gen(StResultsShow) \

#define dGenProcStateEnum(s) s,
dProcessStateEnum(ProcState);

#if 0
#define dGenProcStateString(s) #s,
dProcessStateStr(ProcState);
#endif

using namespace std;

static const char *cReset  = "\033[0m";
static const char *cRed    = "\033[91;1m";
static const char *cYellow = "\033[93;1m";
static const char *cGreen  = "\033[32;1m";
static const char *cCyan   = "\033[96;1m";

static int termWidth()
{
#if defined(_WIN32)
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
		return csbi.srWindow.Right - csbi.srWindow.Left + 1;
	return 80;
#else
	struct winsize ws;
	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0)
		return (int)ws.ws_col;
	return 80;
#endif
}

static int termHeight()
{
#if defined(_WIN32)
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
		return csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
	return 24;
#else
	struct winsize ws;
	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_row > 0)
		return (int)ws.ws_row;
	return 24;
#endif
}

static const char *lineColor(size_t n)
{
	if (n < 100)  return "";
	if (n > 600)  return cRed;
	if (n < 200)  return cYellow;
	if (n > 500)  return cYellow;
	return cGreen;
}

static int lineCategory(size_t n)
{
	if (n < 100)  return 0;   // small
	if (n > 600)  return 2;   // big
	if (n < 200)  return 0;   // small
	if (n > 500)  return 2;   // big
	return 1;                  // green
}

static bool metricsLess(const FileMetrics &a, const FileMetrics &b)
{
	return a.numLines < b.numLines;
}

static size_t gitCommitCount()
{
	FILE *p = popen("git rev-list --count HEAD 2>/dev/null", "r");
	if (!p)
		return 0;
	size_t count = 0;
	if (fscanf(p, "%zu", &count) != 1)
		count = 0;
	pclose(p);
	return count;
}

static bool isGitRepo()
{
	FILE *p = popen("git rev-parse --is-inside-work-tree 2>/dev/null", "r");
	if (!p)
		return false;
	char buf[8] = {};
	bool ok = (fgets(buf, sizeof(buf), p) != NULL);
	pclose(p);
	return ok;
}

MsgDispatching::MsgDispatching()
	: Processing("MsgDispatching")
	//, mStartMs(0)
	, mpSearch(NULL)
{
	mState = StStart;
}

/* member functions */

Success MsgDispatching::process()
{
	//uint32_t curTimeMs = millis();
	//uint32_t diffMs = curTimeMs - mStartMs;
	//Success success;
	Success success;
	bool ok;
#if 0
	dStateTrace;
#endif
	switch (mState)
	{
	case StStart:

		mpSearch = FileSearching::create();
		if (!mpSearch)
			return procErrLog(-1, "could not create FileSearching");

		mpSearch->mExt = env.fileExt;
		start(mpSearch);

		mState = StFileSearchWait;

		break;
	case StFileSearchWait:

		success = mpSearch->success();
		if (success == Pending)
			break;

		if (success != Positive)
			return procErrLog(-1, "file search failed");

		ok = metricsStart();
		if (!ok)
			return procErrLog(-1, "could not start metrics");

		mState = StMetricsWait;

		break;
	case StMetricsWait:

		if (!metricsAllDone())
			break;

		resultsCollect();

		mState = StResultsShow;

		break;
	case StResultsShow:

		resultsPager();

		return Positive;

		break;
	default:
		break;
	}

	return Pending;
}

bool MsgDispatching::metricsStart()
{
	const list<string> &files = mpSearch->mFiles;

	for (list<string>::const_iterator it = files.begin();
			it != files.end(); ++it) {
		const string &f = *it;

		// Filter: skip "main.*" and "*Supervising.*"
		size_t lastSlash = f.rfind('/');
#if defined(_WIN32)
		size_t lastBackslash = f.rfind('\\');
		if (lastBackslash != string::npos &&
				(lastSlash == string::npos || lastBackslash > lastSlash))
			lastSlash = lastBackslash;
#endif
		string basename = (lastSlash == string::npos) ? f : f.substr(lastSlash + 1);
		size_t dot = basename.rfind('.');
		string stem = (dot == string::npos) ? basename : basename.substr(0, dot);

		if (stem == "main")
			continue;
		if (stem.size() >= 10 &&
				stem.substr(stem.size() - 10) == "Supervising")
			continue;

		MetricsGenerating *pMg = MetricsGenerating::create();
		if (!pMg) {
			procWrnLog("could not create MetricsGenerating");
			continue;
		}

		pMg->mFilename = f;
		start(pMg);
		mLstMetrics.push_back(pMg);
	}

	return true;
}

bool MsgDispatching::metricsAllDone()
{
	for (list<MetricsGenerating *>::iterator it = mLstMetrics.begin();
			it != mLstMetrics.end(); ++it) {
		if ((*it)->progress())
			return false;
	}
	return true;
}

void MsgDispatching::resultsCollect()
{
	for (list<MetricsGenerating *>::iterator it = mLstMetrics.begin();
			it != mLstMetrics.end(); ++it) {
		MetricsGenerating *mg = *it;
		if (mg->success() != Positive)
			continue;

		FileMetrics m;
		m.numLines = mg->mNumLines;
		m.license  = mg->mLicense;
		m.name     = mg->mName;
		m.dir      = mg->mDir;
		m.type     = mg->mType;
		mResults.push_back(m);
	}

	sort(mResults.begin(), mResults.end(), metricsLess);
}

void MsgDispatching::resultsPager()
{
	const string &ext = env.fileExt;

	// Build output lines
	vector<string> lines;
	char buf[256];

	size_t numFiles   = mResults.size();
	size_t numProcs   = 0;
	size_t numLibs    = 0;
	size_t numSmall   = 0;
	size_t numGreen   = 0;
	size_t numBig     = 0;
	size_t totalLines = 0;

	lines.push_back("");
	lines.push_back("----------------------------------");

	for (size_t i = 0; i < numFiles; ++i) {
		const FileMetrics &m = mResults[i];
		totalLines += m.numLines;

		const char *col = lineColor(m.numLines);
		int cat = lineCategory(m.numLines);
		if (cat == 0)      ++numSmall;
		else if (cat == 1) ++numGreen;
		else               ++numBig;

		if (m.type == "Process") ++numProcs;
		else if (m.type == "Library") ++numLibs;

		bool hasColor = (col[0] != '\0');
		if (hasColor)
			snprintf(buf, sizeof(buf), "%s%25zu%s   %-8s%-40s%s",
				col,
				m.numLines,
				cReset,
				m.license.c_str(),
				m.name.c_str(),
				m.dir.c_str());
		else
			snprintf(buf, sizeof(buf), "%25zu   %-8s%-40s%s",
				m.numLines,
				m.license.c_str(),
				m.name.c_str(),
				m.dir.c_str());
		lines.push_back(string(buf));
	}

	lines.push_back("----------------------------------");

	snprintf(buf, sizeof(buf), "Lines%s%20zu%s",
		cCyan, totalLines, cReset);
	lines.push_back(string(buf));
	lines.push_back("");

	size_t avg = numFiles ? (totalLines / numFiles) : 0;
	const char *avgCol = lineColor(avg);
	if (avgCol[0] != '\0')
		snprintf(buf, sizeof(buf), "Average%s%18zu%s", avgCol, avg, cReset);
	else
		snprintf(buf, sizeof(buf), "Average%18zu", avg);
	lines.push_back(string(buf));
	lines.push_back("");

	lines.push_back("----------------------------------");
	snprintf(buf, sizeof(buf), "Files%20zu (%s)", numFiles, ext.c_str());
	lines.push_back(string(buf));
	lines.push_back("");

	snprintf(buf, sizeof(buf), "Processes%16zu", numProcs);
	lines.push_back(string(buf));
	snprintf(buf, sizeof(buf), "Libraries%16zu", numLibs);
	lines.push_back(string(buf));
	snprintf(buf, sizeof(buf), "Other%20zu", numFiles - numProcs - numLibs);
	lines.push_back(string(buf));
	lines.push_back("");

	size_t c1 = numFiles ? (numSmall * 100 / numFiles) : 0;
	size_t c2 = numFiles ? (numGreen * 100 / numFiles) : 0;
	size_t c3 = 100 - c1 - c2;

	snprintf(buf, sizeof(buf), "Small%20zu (%zu%%)", numSmall, c1);
	lines.push_back(string(buf));

	// Green percentage coloring
	const char *gcol = cRed;
	if (c2 >= 60) gcol = cGreen;
	else if (c2 >= 35) gcol = cYellow;
	snprintf(buf, sizeof(buf), "Green%20zu (%s%zu%%%s)", numGreen, gcol, c2, cReset);
	lines.push_back(string(buf));

	snprintf(buf, sizeof(buf), "Big%22zu (%zu%%)", numBig, c3);
	lines.push_back(string(buf));
	lines.push_back("");

	// Git commits
	if (isGitRepo()) {
		lines.push_back("----------------------------------");
		size_t commits = gitCommitCount();
		snprintf(buf, sizeof(buf), "Commits%18zu", commits);
		lines.push_back(string(buf));
		lines.push_back("");
	}

	// Show pager
	int h = termHeight();
	int totalL = (int)lines.size();
	// Start at bottom (like less +G)
	int offset = totalL - h + 1;
	if (offset < 0)
		offset = 0;

#if defined(_WIN32)
	// Windows: simple pager using conio
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD oldMode;
	HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
	GetConsoleMode(hIn, &oldMode);
	SetConsoleMode(hIn, ENABLE_WINDOW_INPUT);

	// Enter alternate buffer
	printf("\033[?1049h");
	fflush(stdout);

	while (true) {
		// Render
		printf("\033[H\033[2J");
		for (int i = offset; i < offset + h - 1 && i < totalL; ++i)
			printf("%s\n", lines[(size_t)i].c_str());
		fflush(stdout);

		int c = _getch();
		if (c == 'q' || c == 'Q' || c == 27) break;
		if (c == 'j') {
			if (offset < totalL - h + 1)
				++offset;
		} else if (c == 'k') {
			if (offset > 0)
				--offset;
		} else if (c == 0 || c == 224) {
			int c2 = _getch();
			if (c2 == 80) { // Down arrow
				if (offset < totalL - h + 1)
					++offset;
			} else if (c2 == 72) { // Up arrow
				if (offset > 0)
					--offset;
			}
		}
	}

	SetConsoleMode(hIn, oldMode);
	printf("\033[?1049l");
	fflush(stdout);

#else
	// Unix: pager using termios
	struct termios oldT, newT;
	tcgetattr(STDIN_FILENO, &oldT);
	newT = oldT;
	newT.c_lflag &= ~((tcflag_t)(ICANON | ECHO));
	newT.c_cc[VMIN]  = 1;
	newT.c_cc[VTIME] = 0;
	tcsetattr(STDIN_FILENO, TCSANOW, &newT);

	// Enter alternate screen
	printf("\033[?1049h");
	fflush(stdout);

	(void)termWidth();

	while (true) {
		// Render
		printf("\033[H\033[2J");
		for (int i = offset; i < offset + h - 1 && i < totalL; ++i)
			printf("%s\n", lines[(size_t)i].c_str());
		fflush(stdout);

		// Read key
		char c = 0;
		if (read(STDIN_FILENO, &c, 1) <= 0)
			break;

		if (c == 'q' || c == 'Q')
			break;
		if (c == 27) {
			// Possible escape sequence — read up to 2 more bytes non-blocking
			int oldFlags = fcntl(STDIN_FILENO, F_GETFL, 0);
			fcntl(STDIN_FILENO, F_SETFL, oldFlags | O_NONBLOCK);
			char seq2 = 0, seq3 = 0;
			bool gotSeq = false;
			if (read(STDIN_FILENO, &seq2, 1) > 0 && seq2 == '[') {
				if (read(STDIN_FILENO, &seq3, 1) > 0)
					gotSeq = true;
			}
			fcntl(STDIN_FILENO, F_SETFL, oldFlags);
			if (!gotSeq) {
				break; // plain ESC = quit
			}
			if (seq3 == 'B') { // Down arrow
				if (offset < totalL - h + 1)
					++offset;
			} else if (seq3 == 'A') { // Up arrow
				if (offset > 0)
					--offset;
			}
		} else if (c == 'j') {
			if (offset < totalL - h + 1)
				++offset;
		} else if (c == 'k') {
			if (offset > 0)
				--offset;
		}
	}

	// Restore terminal
	tcsetattr(STDIN_FILENO, TCSANOW, &oldT);
	printf("\033[?1049l");
	fflush(stdout);
#endif
}

void MsgDispatching::processInfo(char *pBuf, char *pBufEnd)
{
	(void)pBuf;
	(void)pBufEnd;
#if 0
	dInfo("State\t\t\t%s\n", ProcStateString[mState]);
#endif
}

/* static functions */

