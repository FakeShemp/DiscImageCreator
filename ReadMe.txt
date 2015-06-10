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
	Sample code path:C:\WinDDK\7600.16385.1\src\storage\tools\spti
	url:http://msdn.microsoft.com/en-us/library/windows/hardware/ff561595(v=vs.85).aspx

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
Apple Macintosh
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
VCD
-----------------------

*Not tested Disc
-----------------------------------------
Acorn Archimedes
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
Vendor					Model					Firmware	Lead-in	Lead-out	Scrambled		GD-ROM					Wii-ROM
---------------------------------------------------------------------------------------------------------------------------------------------------
HITACHI(HL-DT-ST)		GDR-8161B				0045		No		No			No				No						Yes
HITACHI-LG(HL-DT-ST)	GWA-8164B(GDR8164B)		0M08		No		No			No				No						Yes
HITACHI-LG(HL-DT-ST)	GDR-H20N				0D04		No		No			No				No						Yes
HITACHI-LG(HL-DT-ST)	GDR-H20N				0D08		No		No			No				No						Yes
LG(HL-DT-ST)			GCC-4320B				1.00		No		No			Yes				No						No
HP(HL-DT-ST)			GDR-8163B				0B26		No		No			No				No						Yes
HP(TSSTcorp)			TS-H353A				BA08		No		No			No				Yes						No
HP(TSSTcorp)			TS-H353B				bc03(BC05)	No		No			Yes				No						No
LITEON					DH-20A3S				9P56		No		No			Yes				No						No
LITEON					DH-20A3S				9P58		No		No			Yes				No						No
LITEON					LH-18A1P				GL0B		Yes		Yes			No				No						No
LITEON					LH-18A1P				GL0J		Yes		Yes			No				No						No
LITEON					LH-20A1S				9L09		Yes		Yes			No				No						No
LITEON					LTD-163					GDHG		No		No			No				Partial(about 99:59:74)	No
LITEON(JLMS)			LTD-166S(XJ-HD166S)		DS1E		Yes		No			No				No						No
LITEON					SOHW-812S(SOHW-832S)	CG5M		Yes		Yes			No				No						No
NEC						CDR-1700A(286)			3.06		No		No			No				No						No
NEC						CDR-3001A(28F)			3.32		No		No			No				No						No
Optiarc					AD-7203S				1-B0		No		No			No				No						No
PLEXTOR					PX-W1210TA(PX-W1210A)	1.10		Yes		Yes			Yes(only pce)	No						No
PLEXTOR					PX-W2410TA(PX-W2410A)	1.04		Yes		Yes			Yes(only pce)	No						No
PLEXTOR					PX-W8432Ti(PX-W8432T)	1.09		Yes		Yes			Yes(only pce)	No						No
PLEXTOR					PX-320A					1.06		Yes		Yes			Yes(only pce)	No						No
PLEXTOR					PX-504A					1.02		No		No			No				No						No
PLEXTOR					PX-712SA(PX-712A)		1.09		Yes		Yes			Yes				Partial(limit 79:59:73)	No
PLEXTOR					PX-750A					1.03		No		No			No				No						No
PLEXTOR					PX-755SA(PX-755A)		1.08		Yes		Yes			Yes				Partial					No
PLEXTOR					PX-760A					1.07		Yes		Yes			Yes				No						No
Slimtype				DS8A3S					HAT9		No		No			Yes				No						No
TSSTcorp				TS-H192C				HI03		No		No			No				Yes						No
TSSTcorp				TS-H192C				IB01(IB00)	No		No			No				No						No
TSSTcorp				TS-H352C				NE02		No		No			No				Yes						No
TSSTcorp				TS-H492C				IB01		No		No			No				No						No
TSSTcorp				TS-H652C(TS-H652D)		TI06		No		No			No				No						No
TSSTcorp				TS-L162C				DE00		No		No			No				No						No

**Differ offset(with/without subcode reading)
PX-W2410TA(PX-W2410A): +686(with), +98(without)
PX-320A              : +686(with), +98(without)
a bug of firmware? I don't know.

**Attention
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
**2012-03-21
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

**2013-01-03
fix checking & data of subchannel

**2013-01-05
refactoring
improved subchannel data

**2013-01-07
refactoring
add STORAGE ADAPTER DESCRIPTOR log
=> for getting alignmentMask of drive

**2013-01-17
improved PC Engine CD dumping
1. if subchannel and TOC don't sync, TOC has priority.
2. if audio sector exists in data track, their sector is descrambled.

**2013-01-18
fix checking subchannel(RelativeTime)

**2013-01-19
fix ReadTOCText function
=> because mmc1 drive isn't defined.

**2013-01-24
fix subchannel reading
 plextor + audio only disc => read cd command(0xbe) + read subchannel(0x100[pack])
=> because if R-W channel bit is full on, reading speed is very slow.

**2013-01-29
fix cue file of CD-TEXT Disc.

**2013-01-30
fix crach close/stop command.
add OS version log.

**2013-02-03
improved CD-TEXT analyze.
=> support unicode flag

**2013-02-22
add Macintosh disc log.
fix x64 reading.
=>padded with a multiple of four the DataTransferLength of CDROM_READ_TOC_EX_FORMAT_FULL_TOC
fix logic
=>if combined offset is plus and can't read lead-out, tool doesn't work.
  Or if combined offset is minus and can't read lead-in, tool doesn't work.

**2013-03-07
fixed ripping CD-TEXT disc.
improved ripping MCN(EAN) including disc.

**2013-03-09
fixed ccd file (added FLAGS)
improved ripping CD including INDEX 0 in Track 1 disc.

**2013-03-27
fixed ripping DVD DL
fixed ISO9660 PVD log
added DiscInformation(0x51) log
added reading floppy (WIP)

**2013-04-07
added C2 error log (check only one, not re-read)
