#!/bin/sh
for f in ../data_b_115/data/sprite/tutorial/*.button.nut; do
    # out=lang_vi-th175-$(echo $f | tr / - | sed -e 's/data_b_115-//').jdiff

    out=script_latin/th175/$(echo $f | sed -e 's|../data_b_115/||').jdiff
    mkdir -p $(dirname $out)

    ./button_nut_to_jdiff $f > $out
done
