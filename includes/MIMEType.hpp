/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MIMEType.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/07 15:51:01 by mgama             #+#    #+#             */
/*   Updated: 2024/01/08 00:51:25 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <map>

/**
 * Les types de média (MIME) servent à préciser explicitement au client
 * le type de données qu'il reçoit.
 * (https://en.wikipedia.org/wiki/MIME)
 */
class MimeTypes {
private:
    static std::map<std::string, std::string> mimeTypeMapping;

    static std::map<std::string, std::string> initMappings() {
        std::map<std::string, std::string> temp;

		temp["xul"] = "application/vnd.mozilla.xul+xml";
        temp["json"] = "application/json";
        temp["ice"] = "x-conference/x-cooltalk";
        temp["movie"] = "video/x-sgi-movie";
        temp["avi"] = "video/x-msvideo";
        temp["wmv"] = "video/x-ms-wmv";
        temp["m4u"] = "video/vnd.mpegurl";
        temp["mxu"] = "video/vnd.mpegurl";
        temp["htc"] = "text/x-component";
        temp["etx"] = "text/x-setext";
        temp["wmls"] = "text/vnd.wap.wmlscript";
        temp["wml"] = "text/vnd.wap.wml";
        temp["tsv"] = "text/tab-separated-values";
        temp["sgm"] = "text/sgml";
        temp["sgml"] = "text/sgml";
        temp["css"] = "text/css";
        temp["ifb"] = "text/calendar";
        temp["ics"] = "text/calendar";
        temp["wrl"] = "model/vrml";
        temp["vrlm"] = "model/vrml";
        temp["silo"] = "model/mesh";
        temp["mesh"] = "model/mesh";
        temp["msh"] = "model/mesh";
        temp["iges"] = "model/iges";
        temp["igs"] = "model/iges";
        temp["rgb"] = "image/x-rgb";
        temp["ppm"] = "image/x-portable-pixmap";
        temp["pgm"] = "image/x-portable-graymap";
        temp["pbm"] = "image/x-portable-bitmap";
        temp["pnm"] = "image/x-portable-anymap";
        temp["ico"] = "image/x-icon";
        temp["ras"] = "image/x-cmu-raster";
        temp["wbmp"] = "image/vnd.wap.wbmp";
        temp["djv"] = "image/vnd.djvu";
        temp["djvu"] = "image/vnd.djvu";
        temp["svg"] = "image/svg+xml";
        temp["ief"] = "image/ief";
        temp["cgm"] = "image/cgm";
        temp["bmp"] = "image/bmp";
        temp["webp"] = "image/webp";
        temp["xyz"] = "chemical/x-xyz";
        temp["pdb"] = "chemical/x-pdb";
        temp["ra"] = "audio/x-pn-realaudio";
        temp["ram"] = "audio/x-pn-realaudio";
        temp["m3u"] = "audio/x-mpegurl";
        temp["aifc"] = "audio/x-aiff";
        temp["aif"] = "audio/x-aiff";
        temp["aiff"] = "audio/x-aiff";
        temp["mp3"] = "audio/mpeg";
        temp["mp2"] = "audio/mpeg";
        temp["mp1"] = "audio/mpeg";
        temp["mpga"] = "audio/mpeg";
        temp["kar"] = "audio/midi";
        temp["mid"] = "audio/midi";
        temp["midi"] = "audio/midi";
        temp["dtd"] = "application/xml-dtd";
        temp["xsl"] = "application/xml";
        temp["xml"] = "application/xml";
        temp["xslt"] = "application/xslt+xml";
        temp["xht"] = "application/xhtml+xml";
        temp["xhtml"] = "application/xhtml+xml";
        temp["src"] = "application/x-wais-source";
        temp["ustar"] = "application/x-ustar";
        temp["ms"] = "application/x-troff-ms";
        temp["me"] = "application/x-troff-me";
        temp["man"] = "application/x-troff-man";
        temp["roff"] = "application/x-troff";
        temp["tr"] = "application/x-troff";
        temp["t"] = "application/x-troff";
        temp["texi"] = "application/x-texinfo";
        temp["texinfo"] = "application/x-texinfo";
        temp["tex"] = "application/x-tex";
        temp["tcl"] = "application/x-tcl";
        temp["sv4crc"] = "application/x-sv4crc";
        temp["sv4cpio"] = "application/x-sv4cpio";
        temp["sit"] = "application/x-stuffit";
        temp["swf"] = "application/x-shockwave-flash";
        temp["shar"] = "application/x-shar";
        temp["sh"] = "application/x-sh";
        temp["cdf"] = "application/x-netcdf";
        temp["nc"] = "application/x-netcdf";
        temp["latex"] = "application/x-latex";
        temp["skm"] = "application/x-koan";
        temp["skt"] = "application/x-koan";
        temp["skd"] = "application/x-koan";
        temp["skp"] = "application/x-koan";
        temp["js"] = "application/x-javascript";
        temp["hdf"] = "application/x-hdf";
        temp["gtar"] = "application/x-gtar";
        temp["spl"] = "application/x-futuresplash";
        temp["dvi"] = "application/x-dvi";
        temp["dxr"] = "application/x-director";
        temp["dir"] = "application/x-director";
        temp["dcr"] = "application/x-director";
        temp["csh"] = "application/x-csh";
        temp["cpio"] = "application/x-cpio";
        temp["pgn"] = "application/x-chess-pgn";
        temp["vcd"] = "application/x-cdlink";
        temp["bcpio"] = "application/x-bcpio";
        temp["rm"] = "application/vnd.rn-realmedia";
        temp["ppt"] = "application/vnd.ms-powerpoint";
        temp["mif"] = "application/vnd.mif";
        temp["grxml"] = "application/srgs+xml";
        temp["gram"] = "application/srgs";
        temp["smil"] = "application/smil";
        temp["smi"] = "application/smil";
        temp["rdf"] = "application/rdf+xml";
        temp["ogg"] = "application/x-ogg";
        temp["oda"] = "application/oda";
        temp["dmg"] = "application/octet-stream";
        temp["lzh"] = "application/octet-stream";
        temp["so"] = "application/octet-stream";
        temp["lha"] = "application/octet-stream";
        temp["dms"] = "application/octet-stream";
        temp["bin"] = "application/octet-stream";
        temp["mathml"] = "application/mathml+xml";
        temp["cpt"] = "application/mac-compactpro";
        temp["hqx"] = "application/mac-binhex40";
        temp["jnlp"] = "application/jnlp";
        temp["ez"] = "application/andrew-inset";
        temp["txt"] = "text/plain";
        temp["ini"] = "text/plain";
        temp["c"] = "text/plain";
        temp["h"] = "text/plain";
        temp["cpp"] = "text/plain";
        temp["cxx"] = "text/plain";
        temp["cc"] = "text/plain";
        temp["chh"] = "text/plain";
        temp["java"] = "text/plain";
        temp["csv"] = "text/plain";
        temp["bat"] = "text/plain";
        temp["cmd"] = "text/plain";
        temp["asc"] = "text/plain";
        temp["rtf"] = "text/rtf";
        temp["rtx"] = "text/richtext";
        temp["html"] = "text/html";
        temp["htm"] = "text/html";
        temp["zip"] = "application/zip";
        temp["rar"] = "application/x-rar-compressed";
        temp["gzip"] = "application/x-gzip";
        temp["gz"] = "application/x-gzip";
        temp["tgz"] = "application/tgz";
        temp["tar"] = "application/x-tar";
        temp["gif"] = "image/gif";
        temp["jpeg"] = "image/jpeg";
        temp["jpg"] = "image/jpeg";
        temp["jpe"] = "image/jpeg";
        temp["tiff"] = "image/tiff";
        temp["tif"] = "image/tiff";
        temp["png"] = "image/png";
        temp["au"] = "audio/basic";
        temp["snd"] = "audio/basic";
        temp["wav"] = "audio/x-wav";
        temp["mov"] = "video/quicktime";
        temp["qt"] = "video/quicktime";
        temp["mpeg"] = "video/mpeg";
        temp["mpg"] = "video/mpeg";
        temp["mpe"] = "video/mpeg";
        temp["abs"] = "video/mpeg";
        temp["doc"] = "application/msword";
        temp["docx"] = "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
        temp["odt"] = "application/vnd.oasis.opendocument.text";
        temp["xls"] = "application/vnd.ms-excel";
        temp["xlsx"] = "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
        temp["ods"] = "application/vnd.oasis.opendocument.spreadsheet";
        temp["eps"] = "application/postscript";
        temp["ai"] = "application/postscript";
        temp["ps"] = "application/postscript";
        temp["pdf"] = "application/pdf";
        temp["exe"] = "application/octet-stream";
        temp["dll"] = "application/octet-stream";
        temp["class"] = "application/octet-stream";
        temp["jar"] = "application/java-archive";
        return (temp);
    }

public:
    static std::string getMimeType(const std::string& ext) {
        std::map<std::string, std::string>::iterator it = mimeTypeMapping.find(ext);
        if (it != mimeTypeMapping.end()) {
            return it->second;
        }
        /**
         * Par défault nous considérons que le média est un simple texte.
         */
        return "text/plain";
    }
};

std::map<std::string, std::string> MimeTypes::mimeTypeMapping = MimeTypes::initMappings();