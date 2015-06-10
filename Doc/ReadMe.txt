============================= What is this tool? ==============================
* Summary
 This program rips CD, DVD, GD, Floppy. In case of CD, it rips considering a drive offset.
 It works on Windows PC (WinXP or higher).

* Recommend Drive
 CD: PLEXTOR
     (not OEM Drive -> PX-708, PX-712, PX-716, PX-755, PX-760, Premium, PX-5224, PX-4824)
     (Drives must be able to rip by scrambled mode and read lead-out and read lead-in.) 
 GD: http://forum.redump.org/topic/2620/dreamcastnaomi-gdrom-dumping-instructions/
     and TSSTcorp(TS-H353A, TS-H352C, TS-H192C)
 DVD: All supported drive
 Floppy: All supported drive

* Development Tool
** first release - 20131124
 Visual Studio 2010
 Windows Driver Kit(WDK)
  Sample code path: WinDDK\7600.16385.1\src\storage\tools\spti
  url:http://msdn.microsoft.com/en-us/library/windows/hardware/ff561595(v=vs.85).aspx
** 2013-12-17 - latest release
 Visual Studio 2013 (including WDK)

* License
 See License.txt.
 About driveOffset.txt.
  http://www.accuraterip.com/driveoffsets.htm
  Copyright (c) 2015 Illustrate. All Rights Reserved.
 About _external folder
  crc16: http://oku.edu.mie-u.ac.jp/~okumura/algo/
         src\crc16.c in algo.lzh
  crc32: http://www.ietf.org/rfc.html
         using rfc1952 sample code.
  md5: http://www.ietf.org/rfc.html
       using rfc1321 sample code.
  sha1: http://www.ietf.org/rfc.html
        using rfc3174 sample code.

* Disclaimer
 Use this tool at own your risk.
 Trouble in regard to the use of this tool, I can not guarantee any.

* Gratitude
 Thank's redump.org users.

========================== Created files information ==========================
 .bin
  2352byte/sector binary image. This file is used a cue file.
 .c2
  store c2 error. 1bit expresses 1byte.
 .ccd
  store CD information. Original is CloneCD(http://www.slysoft.com/en/clonecd.html)
 .cue
  store CD information. Original is CDRWIN(http://www.goldenhawk.com/cdrwin.htm)
 .dat
  store crc32/md5/sha1 of bin file.
 .img
  2352byte/sector binary image. This file is used a ccd file.
 .scm
  scrambled image file of img file.
 .sub
  store sub channel data. This file is used a ccd file.
 _disclog.txt
  store disc information returned by SCSI command.
 _drivelog.txt
  store drive information returned by SCSI command.
 _errorlog.txt
  store disc error information.
 _infolog.txt
  store volume descriptor, path table, directory table of CD.
 _sub.txt
  store parsed sub channel file as text data.

============================= Ripping information =============================
* Preparation
 Download and install Visual C++ Redistributable Packages.
  http://www.microsoft.com/en-us/download/details.aspx?id=40784
 EccEdc checking tool
  http://www.mediafire.com/download/?7ac64jer16sl7zi/
  Put it to directory of DiscImageCreator.exe.
  (If EccEdc doesn't exist, DiscImageCreator works.)

* Ripping Guide for CD, DVD, Floppy
 Run exe without args to get detail info.

* Ripping Guide for GD-ROM
** Plan1
http://forum.redump.org/topic/2620/dreamcastnaomi-gdrom-dumping-instructions/

** Plan2(The high density area)
1. create the audio trap disc in advance. 
   (a disc with a hacked TOC of 99 mins audio, burn it with CloneCD or Alcohol 52/120%). 
   http://www.mediafire.com/?2nygv2oyzzz
2. insert the audio trap disc to a supported drive.
3. run below. (stop spinning disc)
   DiscImageCreator.exe stop [DriveLetter]
4. use a pin to press the escape eject button, so the tray will eject (or remove
   the drive cover).
5. insert the gdrom and gently push the tray back (or put the drive cover back on).
6. run below. (start rippping gdrom)
   DiscImageCreator.exe gd [DriveLetter] [DriveSpeed(0-72)] foo.bin

========================== Supported/Unsupported Disc =========================
* Supported Disc (I tested these)
 - Apple Macintosh
 - Audio CD
 - Bandai Playdia
 - DVD-Video
 - Fujitsu FM Towns series
 - IBM PC compatible
 - NEC PC-98 series CD
 - NEC PC-Engine CD
 - NEC PC-FX
 - Panasonic 3DO Interactive Multiplayer
 - Philips CD-i
 - Sega Dreamcast
 - Sega Mega-CD
 - Sega Saturn
 - Sony PlayStation
 - Sony PlayStation 2
 - SNK Neo Geo CD
 - VCD

* Support WIP (Because I haven't enough to test)
 - Protected Disc (because I don't have a disc.)
    duplicated(twin) sector
     => Alpha-ROM, ROOT, TAGES
    different frequency
     => SecuROM(v4 or higher), StarForce, CD-Cops
    bad(error) sector
     => SafeDisc, SmartE, Cactus Data Shield 300
    weak sector
     => SafeDisc v2 or higher
    unique data on subchannel
     => PSX PAL, SecuROM(v1)
    no signal sector
     => RingPROTECH, ProRing
    Fake TOC
     => Cactus Data Shield 100
    Intensional(deliberate) C1/C2 error
     => Cactus Data Shield 200

* Supported Disc? (I haven't tested these yet)
 - Acorn Archimedes
 - Bandai / Apple Pippin
 - Commodore Amiga CD
 - Commodore Amiga CD32
 - Commodore Amiga CDTV
 - Mattel HyperScan
 - Microsoft Xbox
 - Microsoft Xbox 360
 - NEC PC-88 series CD
 - Sharp X68000 CD
 - Tandy / Memorex Visual Information System
 - Tao iKTV CD
 - Tomy Kiss-Site CD
 - VM Labs NUON DVD
 - VTech V.Flash

* Unsupported Disc
 - Blu-ray Disc (because I don't have a drive.)
 - HD DVD (because I don't have a drive.)
 - Nintendo GameCube
 - Nintendo Wii (because I don't implement a code to decrypt.)
                (if you have a supported drive, you can rip a "encrypted" image.)

========================= Drive information (I tested) ========================
Vendor					Model					Firmware	Lead-in	Lead-out	Scrambled		GD-ROM					Wii-ROM
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
PLEXTOR					PX-W8432Ti(PX-W8432T)	1.09		Yes		Yes			Yes(only pce)	No						No
PLEXTOR					PX-W1210TA(PX-W1210A)	1.10		Yes		Yes			Yes(only pce)	No						No
PLEXTOR					PX-W2410TA(PX-W2410A)	1.04		Yes		Yes			Yes(only pce)	No						No
PLEXTOR					PX-W4824TA				1.07		Yes		Yes			Yes				No						No
PLEXTOR					PX-W5224A				1.04		Yes		Yes			Yes				No						No
PLEXTOR					PX-320A					1.06		Yes		Yes			Yes(only pce)	No						No
PLEXTOR					PX-504A					1.02		No		No			No				No						No
PLEXTOR					PX-712SA(PX-712A)		1.09		Yes		Yes			Yes				Partial(about 79:59:74)	No
PLEXTOR					PX-750A					1.03		No		No			No				No						No
PLEXTOR					PX-755SA(PX-755A)		1.08		Yes		Yes			Yes				Partial(about 79:59:74)	No
PLEXTOR					PX-760A					1.07		Yes		Yes			Yes				No						No
Slimtype				DS8A3S					HAT9		No		No			Yes				No						No
TSSTcorp				TS-H192C				HI03		No		No			No				Yes						No
TSSTcorp				TS-H192C				IB01(IB00)	No		No			No				No						No
TSSTcorp				TS-H352C				NE02		No		No			No				Yes						No
TSSTcorp				TS-H492C				IB01		No		No			No				No						No
TSSTcorp				TS-H652C(TS-H652D)		TI06		No		No			No				No						No
TSSTcorp				TS-L162C				DE00		No		No			No				No						No

* Attention
 If you rip a GD-ROM, you should rip to internal(and/or NTFS) HDD. 
 Otherwise, if you have a supported drive, you can't only rip about 99:59:74.
 The reason is unknown.
