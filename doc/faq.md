# FAQ: Frequently Asked Questions

**Q: Where does the visualization log stdout information?**

A: The visualization stores stdout generated messages in a visualization.log file in the build directory.

**Q: [WINDOWS] No textures are being loaded!**

A: Check to make sure that the following settings are set in your Visual Studio project:
    - Properties -> Configuration Properties -> Debugging
        - Working Directory = $(SolutionDir)bin\$(Configuration)\