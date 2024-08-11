def column(n):
    # Generate a bitboard for the nth column (0-indexed)
    return sum(1 << (i * 8 + n) for i in range(8))

def columns(*args):
    total = 0
    for arg in args:
        total += column(arg)
    return total
