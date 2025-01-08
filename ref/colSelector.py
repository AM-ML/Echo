def col(n):
    # Generate a bitboard for the nth column (0-indexed)
    return sum(1 << (i * 8 + n) for i in range(8))

def cols(*args):
    total = 0
    for arg in args:
        total += col(arg)
    return total
