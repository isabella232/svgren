README

Building for ios-simulator/iphone-os :
    - pod install in ios folder
    - compile in release for simulator (from the test workspace)
    - compile in release for iphone-os (from the test workspace)
    - Find the Build folder generated in the derived data folder
    - run build.sh script (lipo all the binaries into universal version)
    - update includes if needed.