#!/usr/bin/env python3

"""
# Before you run this script, here is a set of commands
# to generate the input file:

for f in (find . -name '*.nut'); set found (./print-act-nut --print-full-names $f | grep -P -n "[^\x00-\x7F]"); if test "$found" != ""; echo $f; end; end
# Copy list of names
for f in (cat nut_with_sjis.txt); echo $f; ./print-act-nut --print-full-names $f | grep -v Instructions | grep -P "[^\x00-\x7F]"; end > texts.txt
# Edit texts.txt and fix error with multiline strings
cat texts.txt | sed -z 's/\n\([^ .]\)/\\\\n\1/g' | sed -z 's/\n\([^ .]\)/\\\\n\1/g' > texts_escaped_eol.txt
mv texts_escaped_eol.txt texts.txt
"""

fn=None
with open("texts.txt", "r") as fin:
    for line in fin:
        line = line.strip()
        if not ':' in line:
            if line.startswith("./"):
                line = line[2:]
            if line.startswith("data_a/") or line.startswith("data_b/"):
                line = line[len("data_a/"):]
            if fn is None:
                print("""\
<languages />
<translate><!--T:0-->
<!-- Optional message you want to include at the top of the page --></translate>""")
            else:
                print("{{gentext/Footer}}")
            print("""\

{{{{thcrap Game File|{0}}}}}
{{{{gentext/Header}}}}""".format(line))
            fn = line
            n = 1
            continue

        separator = line.find(':')
        key = line[:separator]
        value = line[separator + 1:].strip()
        if value[0] == '"' and value[-1] == '"':
            value = value[1:-1]
        else:
            print(value)
            raise ValueError("Non-quoted value in file")
        print("""{{{{gentext|id={0}|tl=<translate><!--T:{1}_{2}-->
{3}</translate>}}}}""".format(key, fn.replace('/', '_').replace('.', '_'), n, value))
        n += 1

print("""\
{{gentext/Footer}}

{{SubpageCategory}}""")
