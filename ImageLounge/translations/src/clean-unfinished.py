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

If the <translation> is unfinished uses short form tag <translation/> replace with the long form

If the translation is empty and NOT unfinished, make it unfinished
"""

import re
import sys

def process_xml_file(input_file, output_file):
    try:
        with open(input_file, 'r', encoding='utf-8') as f:
            content = f.read()

        modified_content = modify_xml(content)

        with open(output_file, 'w', encoding='utf-8') as f:
            f.write(modified_content)

        print(f"clean-unfinished: Cleaned {input_file} to {output_file}")
    except Exception as e:
        print(f"clean-unfinished: Error processing file: {e}")
        sys.exit(1)

def modify_xml(content):
    # Find all message blocks
    message_pattern = re.compile(r'(<message>.*?</message>)', re.DOTALL)

    def process_message_block(match):
        message_block = match.group(1)

        # Extract source content
        source_match = re.search(r'<source>(.*?)</source>', message_block, re.DOTALL)
        if not source_match:
            return message_block

        source_content = source_match.group(1)

        # Check for translation in various formats
        # 1. Regular translation tag with content
        translation_match = re.search(r'<translation(\s+type="unfinished")?\s*>(.*?)</translation>', message_block, re.DOTALL)
        if translation_match:
            attrs = translation_match.group(1) or ""
            is_unfinished = 'unfinished' in attrs
            translation_content = translation_match.group(2)

            # If source matches translation and translation is unfinished, make it empty
            if source_content.strip() == translation_content.strip() and is_unfinished:
                return message_block.replace(
                    f'<translation{attrs}>{translation_content}</translation>',
                    f'<translation type="unfinished"></translation>'
                )

            # If translation is empty and NOT unfinished, make it unfinished
            if translation_content.strip() == "" and not is_unfinished:
                return message_block.replace(
                    f'<translation>{translation_content}</translation>',
                    f'<translation type="unfinished"></translation>'
                )

        # 2. Handle short form translation
        short_translation_match = re.search(r'<translation(\s+type="unfinished")?\s*/>', message_block)
        if short_translation_match:
            # Always replace short form with long form and ensure it's unfinished
            return message_block.replace(
                short_translation_match.group(0),
                '<translation type="unfinished"></translation>'
            )

        return message_block

    # Process all message blocks
    result = message_pattern.sub(process_message_block, content)
    return result

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print(f"Usage: {sys.argv[0]} input_file output_file")
        sys.exit(1)

    process_xml_file(sys.argv[1], sys.argv[2])
