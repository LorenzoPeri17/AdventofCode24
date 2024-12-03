import numpy as np
import re
from pathlib import Path

def get_result(filename):
    
    _input_pattern = re.compile(r"(do(?:n't)?\(\))|mul\((\d{1,3}),(\d{1,3})\)")
    
    with open(Path(__file__).with_name(filename)) as f:
        lines = ''.join([l.strip('\n') for l in f.readlines()])
    
    matches = _input_pattern.findall(lines)
    result = 0
    enabled = 1
    for match in matches:
        if match[0] == "don't()":
            print('Disabled')
            enabled = 0
        elif match[0] == "do()":
            print('Enabled')
            enabled = 1
        elif match[0] == '':
            print(f'Found mul({match[1]},{match[2]}) and enabled is {enabled}. Result is {result}')
            if enabled:
                result += int(match[1]) * int(match[2])
            
        
    return result

# result = get_result('test2.txt')
# print(f'Test result: {result}')

result = get_result('input.txt')
print(f'The solution to part 2 is: {result}')