import numpy as np
import re
from pathlib import Path

def get_similarity(filename: str):
    
    _input_pattern = re.compile(r'(\d+)\s+(\d+)')

    group1 : list[int] = []
    group2 : list[int] = []

    with open(Path(__file__).with_name(filename)) as f:
        lines = f.readlines()
        for line in lines:
            m = _input_pattern.match(line)
            if m:
                g1, g2 = m.groups()
                group1.append(int(g1))
                group2.append(int(g2))
            else:
                raise ValueError(f'line {line} could not be parsed')
            
    unique, count = np.unique_counts(group2)
    
    counts_g2 : dict[int, int] = {u : c for u, c in zip(unique, count)}
    
    similarity : int = 0
    for g1 in group1:
        similarity += g1 * counts_g2.get(g1,0)
    
    return similarity

test_diff = get_similarity('test.txt')
assert test_diff == 31

input_diff = get_similarity('input.txt')

print(f'The solution to part 2 is: {input_diff}')
