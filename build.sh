if [ $# -lt 1 ]
  then
    echo "Missing parameters!"
    echo ""
    echo "Syntax:"
    echo "   build.sh [DERIVED_DATA_PRODUCTD_PATH]"
    echo ""
    echo "Usage example:"
    echo "  ./build /Users/myuser/Library/Developer/Xcode/DerivedData/tests-dmdhynkqdcmyshfrbigueivlfkfk/Build/Products"
    echo ""
    exit 1
fi


DERIVED_DATA_PRODUCTD_PATH=$1

echo 
echo "DERIVED_DATA_PRODUCTD_PATH: $DERIVED_DATA_PRODUCTD_PATH"


DIST_DIR_IPHONEOS=$DERIVED_DATA_PRODUCTD_PATH/Release-iphoneos
DIST_DIR_IPHONESIM=$DERIVED_DATA_PRODUCTD_PATH/Release-iphonesimulator
# DIST_DIR_IPHONEOS=$DERIVED_DATA_PRODUCTD_PATH/Debug-iphoneos
# DIST_DIR_IPHONESIM=$DERIVED_DATA_PRODUCTD_PATH/Debug-iphonesimulator

declare -a lib_names=("liblibsvgren.a" "liblibcairo.a" "liblibmikroxml.a" "liblibpapki.a" "liblibpixman.a" "liblibpng_igagis.a" "liblibsvgdom.a" "liblibunikod.a")
declare -a folder_names=("libsvgren" "libcairo" "libmikroxml" "libpapki" "libpixman" "libpng_igagis" "libsvgdom" "libunikod")

for i in "${!lib_names[@]}"; do 
	/usr/bin/lipo -create \
	${DIST_DIR_IPHONESIM}/${folder_names[$i]}/${lib_names[$i]} \
	${DIST_DIR_IPHONEOS}/${folder_names[$i]}/${lib_names[$i]} \
	-output ./${lib_names[$i]} 
  printf "%s\t%s\n" "$i" "${lib_names[$i]}"
done
