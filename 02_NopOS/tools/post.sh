#!/bin/bash

nameApp="$1"
nameIso="$2"

dirIso="./iso"
dirGrub="./$dirIso/boot/grub"
cfgGrub="$dirGrub/grub.cfg"

rm -rf "./$dirIso"
mkdir -p $dirGrub

cp "$nameApp" "./$dirIso/boot"

echo "" > $cfgGrub
echo "set timeout = 0" >> $cfgGrub
echo "set default = 0" >> $cfgGrub
echo "" >> $cfgGrub
echo "menuentry \"NopOS\" {" >> $cfgGrub
echo -e "\tmultiboot /boot/$nameApp" >> $cfgGrub
echo -e "\tboot" >> $cfgGrub
echo "}" >> $cfgGrub

# Dependencies
# - xorriso
# - mtools

grub-mkrescue --output="$nameIso" "$dirIso"

