*Summary
This program rips CD/DVD.
Works on Windows PC (I comfirmed Windows 7 64bit).

*Usage
Start cmd.exe & run exe. for more information, run in no arg.
(As far as possible, you should rip to internal HDD. Because USB HDD is very slow.)

*System requirement
Drive that can rip scrambled mode (mainly, "PLEXTOR" drive).
	Recommend:not OEM drive(PX-708A,PX-712A,PX-716A,PX-755,PX-760A)
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
DVD-Video
Fujitsu FM Towns series
IBM PC compatible
NEC PC-98 series CD
NEC PC-Engine CD
NEC PC-FX
Panasonic 3DO Interactive Multiplayer
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
Mattel HyperScan
Microsoft Xbox
Microsoft Xbox 360
NEC PC-88 series CD
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
------------------------------------------------------------------------------
Blu-ray Disc (because I don't have a drive.)
HD DVD (because I don't have a drive.)
Protected Disc (because I don't have a disc.)
 Alpha-ROM, ProRing, SafeDisc, SecuROM, Star Force and so on...
Nintendo GameCube
Nintendo Wii (because I don't implement a code to decrypt.)
             (if you have a supported drive, you can rip a "encrypted" image.)
------------------------------------------------------------------------------

*Tested Drive
Vendor					Model					Firmware	Offset				Lead-in	Lead-out	Scrambled		GD-ROM					Wii-ROM
---------------------------------------------------------------------------------------------------------------------------------------------------
HITACHI(HL-DT-ST)		GDR-8161B				0045		see driveOffset.txt	No		No			No				No						Yes
HITACHI-LG(HL-DT-ST)	GWA-8164B(GDR8164B)		0M08		see driveOffset.txt	No		No			No				No						Yes
HITACHI-LG(HL-DT-ST)	GDR-H20N				0D04		see driveOffset.txt	No		No			No				No						Yes
HITACHI-LG(HL-DT-ST)	GDR-H20N				0D08		see driveOffset.txt	No		No			No				No						Yes
LG(HL-DT-ST)			GCC-4320B				1.00		see driveOffset.txt	No		No			Yes				No						No
HP(HL-DT-ST)			GDR-8163B				0B26		see driveOffset.txt	No		No			No				No						Yes
HP(TSSTcorp)			TS-H353A				BA08		see driveOffset.txt	No		No			No				Yes						No
HP(TSSTcorp)			TS-H353B				bc03(BC05)	see driveOffset.txt	No		No			Yes				No						No
LITEON					DH-20A3S				9P56		see driveOffset.txt	No		No			Yes				No						No
LITEON					DH-20A3S				9P58		see driveOffset.txt	No		No			Yes				No						No
LITEON					LH-20A1S				9L09		see driveOffset.txt	Yes		Yes			No				No						No
LITEON					LTD-163					GDHG		see driveOffset.txt	No		No			No				Partial(about 99:59:74)	No
LITEON(JLMS)			LTD-166S(XJ-HD166S)		DS1E		see driveOffset.txt	Yes		No			No				No						No
LITEON					SOHW-812S(SOHW-832S)	CG5M		see driveOffset.txt	Yes		Yes			No				No						No
NEC						CDR-1700A(286)			3.06		see driveOffset.txt	No		No			No				No						No
NEC						CDR-3001A(28F)			3.32		see driveOffset.txt	No		No			No				No						No
Optiarc					AD-7203S				1-B0		see driveOffset.txt	No		No			No				No						No
PLEXTOR					PX-W1210TA(PX-W1210A)	1.10		see driveOffset.txt	Yes		Yes			Yes(only pce)	No						No
PLEXTOR					PX-W2410TA(PX-W2410A)	1.04		+686 is right		Yes		Yes			Yes(only pce)	No						No
PLEXTOR					PX-W8432Ti(PX-W8432T)	1.09		see driveOffset.txt	Yes		Yes			Yes(only pce)	No						No
PLEXTOR					PX-320A					1.06		+686 is right		Yes		Yes			Yes(only pce)	No						No
PLEXTOR					PX-504A					1.02		see driveOffset.txt	No		No			No				No						No
PLEXTOR					PX-712SA(PX-712A)		1.09		see driveOffset.txt	Yes		Yes			Yes				Partial(limit 79:59:73)	No
PLEXTOR					PX-750A					1.03		see driveOffset.txt	No		No			No				No						No
PLEXTOR					PX-755SA(PX-755A)		1.08		see driveOffset.txt	Yes		Yes			Yes				Partial					No
PLEXTOR					PX-760A					1.07		see driveOffset.txt	Yes		Yes			Yes				No						No
Slimtype				DS8A3S					HAT9		see driveOffset.txt	No		No			Yes				No						No
TSSTcorp				TS-H192C				HI03		see driveOffset.txt	No		No			No				Yes						No
TSSTcorp				TS-H192C				IB01(IB00)	see driveOffset.txt	No		No			No				No						No
TSSTcorp				TS-H352C				NE02		see driveOffset.txt	No		No			No				Yes						No
TSSTcorp				TS-H492C				IB01		see driveOffset.txt	No		No			No				No						No
TSSTcorp				TS-H652C(TS-H652D)		TI06		see driveOffset.txt	No		No			No				No						No
TSSTcorp				TS-L162C				DE00		see driveOffset.txt	No		No			No				No						No

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

*Change Log
**2011-11-xx
**2012-03-xx
**2012-05-26
**2012-06-20
**2012-07-01
**2012-07-07
forgot

**2012-11-02
[DC]
delete .offset .toc file
 => integration .log file
create .gdi file
 => need fix index of track2 manually

[other]
create unicode version

**2012-12-28
support CDG(CD Graphics)
fix Paragraph Boundary

**2012-12-29
fix crashing in x86 build
delete bom in unicode build

**2012-01-03
fix checking & data of subchannel
