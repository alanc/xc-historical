# $XConsortium$
###########################################################################
## Copyright 1990, 1991 by Sun Microsystems, Inc. and the X Consortium.
## 
##                         All Rights Reserved
## 
## Permission to use, copy, modify, and distribute this software and its 
## documentation for any purpose and without fee is hereby granted, 
## provided that the above copyright notice appear in all copies and that
## both that copyright notice and this permission notice appear in 
## supporting documentation, and that the names of Sun Microsystems,
## the X Consortium, and MIT not be used in advertising or publicity 
## pertaining to distribution of the software without specific, written 
## prior permission.  
## 
## SUN MICROSYSTEMS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, 
## INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT 
## SHALL SUN MICROSYSTEMS BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
## DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
## WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
## ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
## SOFTWARE.
## 
###########################################################################
## Finds names that use the same swapping routine and renames them.
##
## Usage:
##	awk -f ReqSame.awk Requests.h > Requests.ci
##
BEGIN {
    num = 0; name = ""; lend = 0; oname = ""; rend = 0; D=0 }
 $1$2 == "#ifdefSWAP_FUNC_PREFIX" {
    D=1
    print "#include \"convReq.h\"" }
 D == 0 {
  if ($4 ~ /SWAP_FUNC_PREFIX/) {
    lend = index($0,"(PEX")
    rend = index($0,")")
    name = substr($0,(lend+4),(rend-lend-4))
    oname = name
    if (name == "FreeLookupTable") {
	name = "GenericRequest" }
    else if (name == "GetDefinedIndices") {
	name = "GenericRequest" }
    else if (name == "FreePipelineContext") {
	name = "GenericRequest" }
    else if (name == "FreeRenderer") {
	name = "GenericRequest" }
    else if (name == "EndRendering") {
	name = "GenericRequest" }
    else if (name == "EndStructure") {
	name = "GenericRequest" }
    else if (name == "CreateStructure") {
	name = "GenericRequest" }
    else if (name == "GetRendererDynamics") {
	name = "GenericRequest" }
    else if (name == "CreateNameSet") {
	name = "GenericRequest" }
    else if (name == "FreeNameSet") {
	name = "GenericRequest" }
    else if (name == "FreeSearchContext") {
	name = "GenericRequest" }
    else if (name == "SearchNetwork") {
	name = "GenericRequest" }
    else if (name == "GetNameSet") {
	name = "GenericRequest" }
    else if (name == "FreePhigsWks") {
	name = "GenericRequest" }
    else if (name == "RedrawAllStructures") {
	name = "GenericRequest" }
    else if (name == "UpdateWorkstation") {
	name = "GenericRequest" }
    else if (name == "ExecuteDeferredActions") {
	name = "GenericRequest" }
    else if (name == "UnpostAllStructures") {
	name = "GenericRequest" }
    else if (name == "GetWksPostings") {
	name = "GenericRequest" }
    else if (name == "FreePickMeasure") {
	name = "GenericRequest" }
    else if (name == "CloseFont") {
	name = "GenericRequest"}
    else if (name == "GetDescendants") {
	name = "GetAncestors" } 
    else {
	name = ""
    }}
  if (name == "") {
    print $0 }
  else {
    print substr($0,1,lend) "PEX" name substr($0,rend,(length($0))) "\t/*" oname "*/" }
  name = "" }

 D == 1 { if ($0 ~ /;/) { D=0 } }
END { }
