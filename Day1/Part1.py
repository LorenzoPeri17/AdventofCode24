import re
from pathlib import Path

def get_difference(filename: str) -> int:
    
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
            
    diff : int = 0
    for g1, g2 in zip(sorted(group1), sorted(group2), strict=True):
        diff += abs(g1-g2)
        
    return diff

test_diff = get_difference('test.txt')
assert test_diff == 11

input_diff = get_difference('input.txt')

print(f'The solution to part 1 is:  {input_diff}')