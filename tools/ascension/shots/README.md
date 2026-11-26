# gbashot — headless screenshots

Boots a ROM in mGBA's emulation core (no display), plays a scripted input sequence and writes PNG screenshots. Used by Claude to check on-screen behaviour (e.g. the Battle Lab) and to post screenshots to the owner's dashboard.

Build (Ubuntu): `apt-get install libmgba-dev libpng-dev && gcc -O2 -Wall -o gbashot gbashot.c -lmgba -lpng`

Script format: `wait N`, `press KEYS [N]`, `hold KEYS N`, `shot NAME`; keys `A B SELECT START RIGHT LEFT UP DOWN R L`, combined with `+` (e.g. `hold L+SELECT 8`). Example: `lab.shots` boots, opens the Battle Lab and starts LAB-001.

Screenshots contain the game's graphics: keep them out of the public repository (they go to the private dashboard).
