./script/fsbl_test/fiptool.py -v genfip \
        'fip.bin' \
        --MONITOR_RUNADDR="0x80040000" \
        --CHIP_CONF='script/fsbl_test/chip_conf.bin' \
        --NOR_INFO='FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF' \
        --NAND_INFO='00000000'\
        --BL2='script/fsbl_test/bl2.bin' \
        --BLCP_IMG_RUNADDR=0x05200200 \
        --BLCP_PARAM_LOADADDR=0 \
        --BLCP=script/fsbl_test/empty.bin \
        --DDR_PARAM='script/fsbl_test/ddr_param.bin' \
        --MONITOR=$1 \
        --compress='lzma'