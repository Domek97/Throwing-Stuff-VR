# Requirements
- [Visual Studio 2022](https://visualstudio.microsoft.com/) (_the free Community edition_)
- [`vcpkg`](https://github.com/microsoft/vcpkg)
  - 1. Clone the repository using git OR [download it as a .zip](https://github.com/microsoft/vcpkg/archive/refs/heads/master.zip)
  - 2. Go into the `vcpkg` folder and double-click on `bootstrap-vcpkg.bat`
  - 3. Edit your system or user Environment Variables and add a new one:
    - Name: `VCPKG_ROOT`  
      Value: `C:\path\to\wherever\your\vcpkg\folder\is`

# To build
-Clone this repository using git OR download it as a .zip
-Replace the code in "root/build/debug/vcpkg_installed/x64-windows-static/include/RE/M/MagicTarget.h" with the version in [CommonLibVR](https://github.com/beef-buns/CommonLibVR/blob/c137dd1c6a8d94fc102da651106006aeac416b55/include/RE/M/MagicTarget.h)