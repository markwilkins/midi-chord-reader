#!/bin/bash

# I lifted these commands and basics from this Juce forum post:
# https://forum.juce.com/t/vst-installer/16654/15?page=2

name="MidiChords"
version="0.8.0"

mkdir au
cp -R ~/Library/Audio/Plug-Ins/Components/$name.component au

mkdir vst3/
cp -R ~/Library/Audio/Plug-Ins/VST3/${name}.vst3 vst3

pkgbuild --analyze --root "./vst3/" "${name}_VST3.plist" \

pkgbuild --analyze --root "./au/" "${name}_AU.plist"

pkgbuild --root "./vst3/" --component-plist "./${name}_VST3.plist" --identifier "com.tetrachord.pkg.VST3" --version $version --install-location "/Library/Audio/Plug-Ins/VST3" "${name}_VST3.pkg"

pkgbuild --root "./au/" --component-plist "./${name}_AU.plist" --identifier "com.tetrachord.pkg.AU" --version $version --install-location "/Library/Audio/Plug-Ins/Components" "${name}_AU.pkg"


# Need to work on this ... and maybe have to pay for apple dev membership. Or look at:
#         https://security.stackexchange.com/questions/17909/how-to-create-an-apple-installer-package-signing-certificate
# productbuild --distribution "./Distribution.xml" --package-path "./" --resources "./Resources" --sign "Developer ID Installer: Tetrachord" "${name}_${version}.pkg"
# This builds it without signing. 
installfile="${name}_${version}.pkg"
productbuild --distribution "./Distribution.xml" --package-path "./" --resources "./Resources" ${installfile}

# build a file with the checksums using md5 and sha256 for verification
md5hashvalue=$(md5 ${installfile} | awk '{print $4}')
shahashvalue=$(shasum -a 256 ${installfile} | awk '{print $1}')
echo "Checksum of ${installfile}" > hashes.txt
echo "MD5: ${md5hashvalue}" >> hashes.txt
echo "SHA256: ${shahashvalue}" >> hashes.txt

# move the install image stuff
mkdir -p ../instimages
mv ${installfile} ../instimages
mv hashes.txt ../instimages


# cleanup
rm "${name}_VST3.plist"
rm "${name}_AU.plist"
rm "${name}_VST3.pkg"
rm "${name}_AU.pkg"
rm -r "./vst3"
rm -r "./au"
