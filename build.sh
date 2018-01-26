DIST_DIR_IPHONEOS=ios/Build/Products/Release-iphoneos
DIST_DIR_IPHONESIM=ios/Build/Products/Release-iphonesimulator

declare -a lib_names=("liblibsvgren.a" "liblibcairo.a" "liblibmikroxml.a" "liblibpapki.a" "liblibpixman.a" "liblibpng_igagis.a" "liblibsvgdom.a" "liblibunikod.a")
declare -a folder_names=("libsvgren" "libcairo" "libmikroxml" "libpapki" "libpixman" "libpng_igagis" "libsvgdom" "libunikod")

for i in "${!lib_names[@]}"; do 
	/usr/bin/lipo -create \
	${DIST_DIR_IPHONESIM}/${folder_names[$i]}/${lib_names[$i]} \
	${DIST_DIR_IPHONEOS}/${folder_names[$i]}/${lib_names[$i]} \
	-output ./${lib_names[$i]} 
  printf "%s\t%s\n" "$i" "${lib_names[$i]}"
done
