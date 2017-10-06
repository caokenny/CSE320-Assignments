# This Python file uses the following encoding: utf-8
import codecs
unicodeString = u"😈👩‍🚀💩👻💀☠️👽👾🤖🎃"
out = file( "./UTF8.txt", "w" )
out_utf16be = file( "./UTF16BE.txt", "w" )
out_utf16le = file( "./UTF16LE.txt", "w" )


out.write( codecs.BOM_UTF8 )
out.write( unicodeString.encode( "utf-8" ) )

out_utf16be.write(codecs.BOM_BE);
out_utf16be.write(unicodeString.encode("utf_16_be"))

out_utf16le.write(codecs.BOM_LE);
out_utf16le.write(unicodeString.encode("utf_16_le"))


out.close()
out_utf16be.close()
out_utf16le.close()