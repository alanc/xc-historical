#
# sample font server configuration file
#
clone-self = off
cache-size = 1000000
client-limit = 10
use-syslog = off
alternate-servers = hansen:1,hansen:2
catalogue = pcf:/u14/lemke/fonts,pcf:/u14/lemke/fontserver/fonts/data/pcf,sp:/u14/lemke/fontserver/fonts/data/speedo
#catalogue = pcf:/u14/lemke/fonts,sp:/u14/lemke/fontserver/fonts/data/speedo
#catalogue = pcf:/u14/lemke/fontserver/fonts/data/pcf,fs:hansen:0,snf:/usr/lib/X11/fonts/misc
error-file = /tmp/fs-errors

# in decipoints
default-point-size = 120
default-resolutions = 72,72,100,100
