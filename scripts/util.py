def flip(x):
    res = list(bin(x)[2:])
    while len(res) != 8:
        res.insert(0, '0')
    for i in range(len(res)):
        if res[i] == '0':
            res[i] = '1'
        else:
            res[i] = '0'
    return '0b' + ''.join(res)

def describe(x):
    for i in range(8):
        print(f'{7-i}: {x & (1 << (7-i)) > 0}')
