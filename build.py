import re
from pathlib import Path


VERSION = "0.0.1"

INCLUDE_DIR = Path("include")
MAIN_FILE = "dcf.hpp"
OUT_FILE = Path("dist/dcf.hpp")



def include(file, already_included):
    comment_border = "/*---------------------------*/"
    lines = ["\n", comment_border, f"//   begin of \"{file}\"",  comment_border]

    with (INCLUDE_DIR / file).open("r") as f:
        for line in f:
            match = re.match(r'#include\s+"(.+?)"', line.strip())
            if match:
                included_file = match.group(1)
                if included_file not in already_included:
                    already_included.add(included_file)
                    lines.append(include(included_file, already_included))
            elif line.strip():
                lines.append(line.strip("\n"))

    lines.extend([comment_border, f"//   end of \"{file}\"", comment_border, ""])
    return "\n".join(lines)




def main():
    final = include(MAIN_FILE, already_included={MAIN_FILE})
    OUT_FILE.parent.mkdir(parents=True, exist_ok=True)
    OUT_FILE.write_text(f"""/*+++++++++++++++++++++++++++*/
//   version {VERSION}
/*+++++++++++++++++++++++++++*/

{final}
""")
    print(f"Built {INCLUDE_DIR / MAIN_FILE} to {OUT_FILE} as version {VERSION}")



if __name__ == "__main__":
    main()
