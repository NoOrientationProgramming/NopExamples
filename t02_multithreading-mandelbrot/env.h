/*
  This file is part of the DSP-Crowd project
  https://www.dsp-crowd.com

  Author(s):
      - Johannes Natter, office@dsp-crowd.com

  File created on 31.03.2018

  Copyright (C) 2018, Johannes Natter
*/

#ifndef ENV_H
#define ENV_H

#include <string>

/*
 * ##################################
 * Environment contains variables for
 *          !! IPC ONLY !!
 * ##################################
 */

struct Environment
{
	bool haveTclap;
	bool daemonDebug;
	int verbosity;
#if defined(__unix__)
	bool coreDump;
#endif
	std::string nameFile;
	std::string dirOutput;
	bool forceDouble;
#if APP_HAS_AVX2
	bool disableSimd;
#endif
	uint16_t port;

	uint32_t imgWidth;
	uint32_t imgHeight;
	size_t numIterMax;
	double posX;
	double posY;
	double zoom;
};

extern Environment env;

#endif

