/* XML shared by the ODT writer overlays (8 and 11) */
static const char x_head[] =
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
static const char x_ns[] =
    " xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\""
    " xmlns:style=\"urn:oasis:names:tc:opendocument:xmlns:style:1.0\""
    " xmlns:text=\"urn:oasis:names:tc:opendocument:xmlns:text:1.0\""
    " xmlns:fo=\"urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0\""
    " xmlns:svg=\"urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0\""
    " office:version=\"1.2\">";
static const char x_fonts[] =
    "<office:font-face-decls>"
    "<style:font-face style:name=\"Liberation Sans\" svg:font-family=\"&apos;Liberation Sans&apos;\" style:font-family-generic=\"swiss\" style:font-pitch=\"variable\"/>"
    "<style:font-face style:name=\"Liberation Serif\" svg:font-family=\"&apos;Liberation Serif&apos;\" style:font-family-generic=\"roman\" style:font-pitch=\"variable\"/>"
    "<style:font-face style:name=\"Liberation Mono\" svg:font-family=\"&apos;Liberation Mono&apos;\" style:font-family-generic=\"modern\" style:font-pitch=\"fixed\"/>"
    "</office:font-face-decls>";

