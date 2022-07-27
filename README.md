# ToolDaemon
------------

Mac OS Classic shareware made open source


ToolDaemon makes your (classic) MacOS computer remotely accessible to authorized users, from any platform, through any Telnet client.

Indeed, your (classic) MacOS computer becomes a true multi-threaded, multi-user environment, where logged users can simultaneously take *any* action on the system.

To achieve this, ToolDaemon needs some help from ToolServer, a very nice program from Apple Computer, Inc.  


## Building
-----------

Prerequsites:

* CodeWarrior Pro 5.3
* AppleShare API
* ICProgKit 202
* Toast Titanium
* Stuffit Expander
* ResEdit


Clone the repository and copy the resource forks back:

```console
cp AppResources.ppob_..namedfork_rsrc AppResources.ppob/..namedfork/rsrc
cp AppResources.rsrc_..namedfork_rsrc AppResources.rsrc/..namedfork/rsrc
```

Set up a build directory such as:

* Macintosh HD
  * Projects
    * APIs
    * ToolDaemon Folder
      * ToolDaemon

In CodeWarrior, the variable {Project} will refer to the "Tool Daemon" folder.  APIs are referenced as {Project}:::APIs:

Use ResEdit to set the Type and Creator codes for the following files:

{Project}ToolDaemon.mcp
* Type: MMPr
* Creator: CWIE

{Project}Resources:AppResources.ppob
* Type: rsrc
* Creator: MWC2

{Project}Resources:AppResources.rsrc
* Type: rsrc
* Creator: RSED

{Project}Source:Prefix:DebugPrefix.pch++
* Type: TEXT
* Creator: CWIE

{Project}Source:Prefix:FinalPrefix.pch++
* Type: TEXT
* Creator: CWIE


**AppleShare API:**
* You can get the AppleShare API from Macintosh Repository.  This link is for the October 1997 SDK's CD from the Developer Connection:
https://www.macintoshrepository.org/download.php?id=26091

* Mount Dev.CD Oct 97 SDK1.toast using Toast and copy the "AppleShare API" folder to the APIs folder you created above.


**InternetConfig SDK:**
* Get the InternetConfig SDK from here:
http://dev.macperl.org/files/macperl_build_libs/icprogkit-202-source.sit.bin

* Extract it and place the resulting "ICProgKit2.0.2" folder into the APIs folder.


**Build the MSL Libraries:**
* Open the folder:
```
Macintosh HD:CodeWarrior Pro 5:Metrowerks CodeWarrior:MSL:(MacOS_Build_Scripts)
```

* Launch "Build MSL Libraries"
Build at least all of the PPC libraries, and all the 68K libraries if you plan on running ToolDaemon on a 68K machine as well.

You may need to increase the memory allocation for CodeWarrior IDE if you get build errors.


**Build ToolDaemon:**

Open the ToolDaemon.mcp project file.

Click the "Make" button.

