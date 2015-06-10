*Summary
This program rips CD/DVD.
Works on Windows PC (I comfirmed Windows 7 64bit).

*Usage
Start cmd.exe & run exe. for more information, run in no arg.
(As far as possible, you should rip to internal HDD. Because USB HDD is very slow.)

*System requirement
Drive that can rip scrambled mode (mainly, "PLEXTOR" drive).
In case of GD-ROM, refer to "tested drive".

*Development Tool
Visual Studio 2010
Windows Driver Kit(WDK)

*License
See License.txt.
About driveOffset.txt.
 http://www.accuraterip.com/driveoffsets.htm
 Copyright (c) 2012 Illustrate. All Rights Reserved.

*Disclaimer
Use this tool at own your risk.
Trouble in regard to the use of this tool, I can not guarantee any.

*Tested Disc
-----------------------
Audio CD
Fujitsu FM Towns series
IBM PC compatible
NEC PC-98 series CD
NEC PC-Engine CD
NEC PC-FX
Sega Dreamcast
Sega Mega-CD
Sega Saturn
Sony PlayStation
Sony PlayStation 2
SNK Neo Geo CD
-----------------------

*Not tested Disc
-----------------------------------------
Acorn Archimedes
Apple Macintosh
Bandai Playdia
Bandai / Apple Pippin
Commodore Amiga CD
Commodore Amiga CD32
Commodore Amiga CDTV
DVD-Video
Mattel HyperScan
Microsoft Xbox
Microsoft Xbox 360
NEC PC-88 series CD
Panasonic 3DO Interactive Multiplayer
Philips CD-i
Sharp X68000 CD
Tandy / Memorex Visual Information System
Tao iKTV CD
Tomy Kiss-Site CD
VCD
VM Labs NUON DVD
VTech V.Flash
-----------------------------------------

*Unsupported Disc
----------------------------------------------------------------------------
Blu-ray Disc (because I don't have a drive.)
HD DVD (because I don't have a drive.)
Protected Disc (because I don't have a disc.)
 Alpha-ROM, ProRing, SafeDisc, SecuROM, Star Force and so on...
Nintendo GameCube
Nintendo Wii (because I don't implement a code to decrypt.)
             (if you have a supported drive, you can rip a "encrypted" image.)
----------------------------------------------------------------------------

*Tested Drive
Vendor					Model					Firmware	Lead-in	Lead-out	Scrambled	GD-ROM									Wii-ROM
-------------------------------------------------------------------------------------------------------------------------------------------
HITACHI(HL-DT-ST)		GDR-8161B				0045		No		No			No			No(not open drive cover)				Yes
HITACHI-LG(HL-DT-ST)	GWA-8164B(GDR8164B)		0M08		No		No			No			No(not open drive cover)				Yes
HITACHI-LG(HL-DT-ST)	GDR-H20N				0D04		No		No			No			No(not open drive cover)				Yes
LG(HL-DT-ST)			GCC-4320B				1.00		No		No			Yes			No(not open drive cover)				No
HP(HL-DT-ST)			GDR-8163B				0B26		No		No			No			No(not open drive cover)				Yes
HP(TSSTcorp)			TS-H353A				BA08		No		No			No			Yes										No
HP(TSSTcorp)			TS-H353B				bc03(BC05)	No		No			Yes			No(not open drive cover)				No
LITEON					DH-20A3S				9P56		No		No			Yes			No(not open drive cover)				No
LITEON					DH-20A3S				9P58		No		No			Yes			No(not open drive cover)				No
LITEON					LH-20A1S				9L09		Yes		Yes			No			No(not open drive cover)				No
LITEON					LTD-163					GDHG		No		No			No			Partial(about 99:59:74)					No
LITEON(JLMS)			LTD-166S(XJ-HD166S)		DS18		Yes		No			No			No(not open drive cover)				No
LITEON(JLMS)			LTD-166S(XJ-HD166S)		DS1E		Yes		No			No			No(not open drive cover)				No
LITEON					SOHW-812S(SOHW-832S)	CG5M		Yes		Yes			No			No(not open drive cover)				No
NEC						CDR-1700A(286)			3.06		No		No			No			No(not open drive cover)				No
NEC						CDR-3001A(28F)			3.32		No		No			No			No(not open drive cover)				No
Optiarc					AD-7203S				1-B0		No		No			No			No(not open drive cover)				No
PLEXTOR					PX-W1210TA(PX-W1210TA)	1.10		Yes		Yes			Yes			No(not open drive cover)				No
PLEXTOR					PX-W8432T				1.09		Yes		Yes			Yes			No(not open drive cover)				No
PLEXTOR					PX-320A					1.06		Yes		Yes			Yes			No(not open drive cover)				No
PLEXTOR					PX-504A					1.02		No		No			No			No(not open drive cover)				No
PLEXTOR					PX-712SA(PX-712A)		1.09		Yes		Yes			Yes			Partial(no swap disc)(limit 79:59:73)	No
PLEXTOR					PX-755SA(PX-755A)		1.08		Yes		Yes			Yes			No(not open drive cover)				No
Slimtype				DS8A3S					HAT9		No		No			Yes			No(not open drive cover)				No
TSSTcorp				TS-H192C				HI03		No		No			No			Yes										No
TSSTcorp				TS-H192C				IB01(IB00)	No		No			No			No(not open drive cover)				No
TSSTcorp				TS-H352C				NE02		No		No			No			Yes										No
TSSTcorp				TS-H492C				IB01		No		No			No			No(not open drive cover)				No
TSSTcorp				TS-H652C(TS-H652D)		TI06		No		No			No			No(not open drive cover)				No
TSSTcorp				TS-L162C				DE00		No		No			No			No(not open drive cover)				No

**attention
if you rip a GD-ROM, you should rip to internal(and/or NTSC) HDD. 
Otherwise, if you have a supported drive, you can't only rip about 99:59:74.
The reason is unknown.
-------------------------------------------------------------------------------------------------------------------------------------------

*Ripping Guide for GD-ROM
**Plan1
http://forum.redump.org/topic/2620/dreamcastnaomi-gdrom-dumping-instructions/

**Plan2(The high density area)
1. create the audio trap disc in advance. 
   (a disc with a hacked TOC of 99 mins audio, burn it with CloneCD or Alcohol). 
   http://www.mediafire.com/?2nygv2oyzzz
2. insert the audio trap disc to a supported drive.
3. run below. (stop disc)
   DiscImageCreator.exe -s [DriveLetter]
4. use a pin to press the escape eject button, so the tray will eject (or remove the drive cover).
5. insert the gdrom and gently push the tray back (or put the drive cover back on).
6. run below. (rip GD-ROM)
   DiscImageCreator.exe -ra [DriveLetter] [DriveSpeed(0-72)] foo.bin 44990 549150
7. run below. (descramble)
   DiscImageCreator.exe -dec foo.bin 44990
8. run below. (according to toc(0x110-0x297), split .dec)
   DiscImageCreator.exe -split foo.dec
