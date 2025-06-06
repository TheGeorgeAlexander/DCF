import re
import os


version = "0.0.1"

include_dir = "include/"
main_file = "dcf.hpp"
out_file = "dist/dcf.hpp"



def main():
    final = include(main_file)
    os.makedirs(os.path.dirname(out_file), exist_ok=True)
    with open(out_file, "w+") as out:
        out.write(f"""/*+++++++++++++++++++++++++++*/
//   version {version}
/*+++++++++++++++++++++++++++*/

""" + final)
    print(f"Built {include_dir}{main_file} to {out_file} as version {version}")




already_included = {main_file}
def include(file):
    final = f"""
/*---------------------------*/
//   begin of "{file}"
/*---------------------------*/
"""
    for line in open(include_dir + file, "r"):
        regex_match = re.match("#include \"(.+?)\"", line.strip())
        if regex_match != None:
            included_file = regex_match.group(1)
            if included_file not in already_included:
                already_included.add(included_file)
                final += include(included_file)
        elif line.strip() != "":
            final += line
            if final[-1] != "\n":
                final += "\n"
    return final + f"""/*---------------------------*/
//   end of "{file}"
/*---------------------------*/

"""



if __name__ == "__main__":
    main()
