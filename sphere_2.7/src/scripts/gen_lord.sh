#!/bin/sh

sed -e "s/-lsp/-lsp_7/" \
        -e "s/-lutil/-lutil_9/" \
        -e "s/-lgp/-lgp_9/" \
        -e "s/-lsnr/-lsnr_5/" \
        -e "s/-lphone/-lphone_5/" \
        -e "s/-lplex/-lplex_5/" \
        -e "s/^\(-[a-z]*\)$/\1_1/"
