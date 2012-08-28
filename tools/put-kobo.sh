#!/bin/sh

( sleep 1; echo "root"; echo "cd /tmp"; echo "lcd build"; echo "put ocher"; echo "quit" ) | ftp 192.168.1.68

