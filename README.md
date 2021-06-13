ROOTMAP Core Test repo - unit tests, sample app for facilitating ROOTMAP.Core development, and eventually integration tests. References ROOTMAP.Core repo as a git submodule.

## TESTING

ROOTMAP.Core.Test uses [Catch2](https://github.com/catchorg/Catch2) for unit testing. To run tests in Visual Studio, it uses [TestAdapter_Catch2](https://github.com/JohnnyHendriks/TestAdapter_Catch2).

### Testing Quickstart

* Install Test Adapter for Catch2 in Visual Studio per [the instructions](https://github.com/JohnnyHendriks/TestAdapter_Catch2/blob/master/README.md)
* Open the ROOTMAP.Core solution in VS
* Select the test settings file: **Test --> Test Settings --> Select Test Settings File**
* Select `CatchTests.runsettings`
* Select **Debug** build configuration and build the solution
* Run tests: **Test --> Run --> All Tests**
* Observe the output in the **Test Explorer** window (you may need to open this window: **Test --> Windows --> Test Explorer**)
