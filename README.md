Transit README

# Infos
This repo is a fork of `https://github.com/igagis/svgren`. 

The repo contains those changes:

* Files from the `svgdom` repo (https://github.com/igagis/svgdom) have been copied in the directory `svgdom`.
* It contains built libraries files (ex: liblibsvgdom.a, liblibsvgren.a, ...)



# How to build libraries files for Transit app

Building for ios-simulator/iphone-os:

* From the `ios` directory run: `pod install`
* Open the workspace `tests.xcworkspace`
	- Select a Simulator, and run the project. The build must be in Release configuration
	- Select a iOS Device, and run the project. The build must be in Release configuration
* Find the `Products` folder generated in the derived data folder. To find it, from the Xcode Project Navigation, right-click `Products/tests.app` and select *Show in finder*.
* run build.sh script using the `Products` folder name. The script lipo all the binaries into universal version.
	- ex: `./build.sh /Users/myser/Library/Developer/Xcode/DerivedData/tests-dmdhynkqdcmyshfrbigueivlfkfk/Build/Products`
* Commit your changes
