if request.isInit:
    part_id = 0x15
    rev_id = 0x00

elif request.isRead:
    if request.offset == 0xFF:
        request.value = part_id
    elif request.offset == 0xFE:
        request.value = rev_id
    else:
        request.value = 0x00

elif request.isWrite:
    pass

self.NoisyLog("MAX30102 I2C %s at 0x%x, value 0x%x" % (str(request.type), request.offset, request.value))
