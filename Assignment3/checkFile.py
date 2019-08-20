import sys
def diff_error(fileA,fileB):
    with open(fileA, "r") as f:
        dataA = f.readlines()
    with open(fileB, "r") as f:
        dataB = f.readlines()
    linesA = len(dataA)
    linesB = len(dataB)
    if linesA != linesB:
        return False
    retFlag = True
    for index in range(linesA):
        if dataA[index] != dataB[index]:
            xa, ya = dataA[index].split()
            xb, yb = dataB[index].split()
            xa = float(xa)
            xb = float(xb)
            ya = float(ya)
            yb = float(yb)
            if abs(xa-xb) > 5:
                print('Error x: on line %d -> %f %f'%(index,xa,xb))
                #return False
                retFlag = False
            if abs(ya-yb) > 5:
                print('Error y: on line %d -> %f %f'%(index,ya,yb))
                #return False
                retFlag = False
    return retFlag

print("Comparing : %s and %s"%(sys.argv[1],sys.argv[2]))
if diff_error(sys.argv[1],sys.argv[2]):
    print("OK: PASS!")
else:
    print("NOT OK: Files are not match")