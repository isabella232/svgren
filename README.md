README

Building for ios-simulator/iphone-os :
    - pod install in ios folder
    - compile in release for simulator
    - compile in release for iphone-os
    - run build.sh script (lipo all the binaries into universal version)
    - copie all the .a generates into the transitLib/svgren root folder.
    - update includes if needed.