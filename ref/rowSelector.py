def row(n):
    return (2**(8*n)-1)

def rows(*args):
    sum = 0
    for arg in args:
        sum += row(arg) - row(arg-1)
    return sum

