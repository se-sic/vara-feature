import sys

with open('CMakeLists.txt', 'r') as f:
    lines = f.readlines()

# Find the set(Z3_INCLUDE_DIR line
for i in range(len(lines)):
    if 'set(Z3_INCLUDE_DIR' in lines[i]:
        start_idx = i
        # Find the closing parenthesis
        paren_count = 0
        for j in range(i, len(lines)):
            line = lines[j]
            paren_count += line.count('(')
            paren_count -= line.count(')')
            if 'set(Z3_INCLUDE_DIR' in line:
                paren_count = 1  # Reset for this set command
            if paren_count == 0 and j > i:
                end_idx = j
                break
        
        # Now replace lines[start_idx:end_idx+1]
        replacement = [
            '    # First compute the concatenated path\n',
            '    string(CONCAT Z3_CONCAT_PATH \n',
            '           "${PROJECT_BINARY_DIR}/${VARA_FEATURE_IN_TREE_PREFIX}"\n',
            '           "external/z3/src/api")\n',
            '    # Then set the variable\n',
            '    set(Z3_INCLUDE_DIR\n',
            '        ${Z3_CONCAT_PATH}\n',
            '        "${CMAKE_CURRENT_SOURCE_DIR}/external/z3/src/api"\n',
            '        "${CMAKE_CURRENT_SOURCE_DIR}/external/z3/src/api/c++"\n',
            '    )\n'
        ]
        
        # Replace the block
        lines = lines[:start_idx] + replacement + lines[end_idx+1:]
        break

# Write back
with open('CMakeLists.txt', 'w') as f:
    f.writelines(lines)

print("Fixed the Z3_INCLUDE_DIR set command")
