#!/usr/bin/env python3

# Claude 3.7 Thinking

"""
I want a python script to modify xml files with the format I'm going to give you.
The formatting should be exactly the same except for one change.

If within a <message> block, the <source> content matches the <translation> content,
and the <translation> is marked as "unfinished", then the <translation> should be rewritten
to be empty and remain unfinished. All other formatting must remain identical.

You may not use a general xml parser for this. Here is the sample xml you will need to modify.

<message>
  <location filename="../src/DkCore/DkActionManager.cpp" line="1148"/>
  <source>&amp;90%1 Counter Clockwise</source>
  <translation>&amp;90%1 ضد عقارب الساعة</translation>
</message>

Please check your script supports multiline text in the <source> and <translation>
"""

import re
import sys

def process_ts_file(input_file, output_file=None):
    if output_file is None:
        output_file = input_file

    try:
        with open(input_file, 'r', encoding='utf-8') as f:
            content = f.read()
    except Exception as e:
        print(f"Error reading file {input_file}: {e}")
        return False

    # Pattern to find complete <message> blocks
    message_pattern = re.compile(r'(<message.*?>.*?</message>)', re.DOTALL)
    
    # Function to process each <message> block
    def process_message(match):
        message_block = match.group(1)
        
        # Extract source content with regex that properly handles multiline content
        source_match = re.search(r'<source>(.*?)</source>', message_block, re.DOTALL)
        if not source_match:
            return message_block
        
        source_content = source_match.group(1)
        
        # Look for unfinished translations
        translation_match = re.search(r'(<translation\s+type="unfinished">)(.*?)(</translation>)', message_block, re.DOTALL)
        if translation_match:
            translation_start = translation_match.group(1)
            translation_content = translation_match.group(2)
            translation_end = translation_match.group(3)
            
            # Compare content after stripping whitespace
            if translation_content.strip() == source_content.strip():
                # Rebuild the message block with empty translation content
                start_pos = message_block.find(translation_match.group(0))
                end_pos = start_pos + len(translation_match.group(0))
                
                modified_block = (
                    message_block[:start_pos] + 
                    translation_start + 
                    "" +  # Empty content
                    translation_end + 
                    message_block[end_pos:]
                )
                return modified_block
        
        return message_block

    # Process all message blocks
    modified_content = message_pattern.sub(process_message, content)
    
    # Write modified content to output file
    try:
        with open(output_file, 'w', encoding='utf-8') as f:
            f.write(modified_content)
        print(f"File processed successfully: {output_file}")
        return True
    except Exception as e:
        print(f"Error writing to file {output_file}: {e}")
        return False

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: %s input_file.ts [output_file.ts]" % sys.argv[0])
        sys.exit(1)
    
    input_file = sys.argv[1]
    output_file = sys.argv[2] if len(sys.argv) > 2 else input_file
    
    if process_ts_file(input_file, output_file):
        sys.exit(0)
    else:
        sys.exit(1)
