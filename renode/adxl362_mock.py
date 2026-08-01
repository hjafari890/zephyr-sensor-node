if request.isInit:
    devid_ad = 0xAD
    devid_mst = 0x1D
    part_id = 0xF2
    rev_id = 0x01

elif request.isRead:
    if request.offset == 0x00:
        request.value = devid_ad
    elif request.offset == 0x01:
        request.value = devid_mst
    elif request.offset == 0x02:
        request.value = part_id
    elif request.offset == 0x03:
        request.value = rev_id
    else:
        request.value = 0x00

elif request.isWrite:
    pass

self.NoisyLog("ADXL362 SPI %s at 0x%x, value 0x%x" % (str(request.type), request.offset, request.value))
