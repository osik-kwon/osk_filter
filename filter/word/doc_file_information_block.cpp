#include "filter_pch.h"
#include "word/doc_file_information_block.h"

namespace filter
{
namespace doc
{
	FileInformationBlock::FileInformationBlock()
	{}

	// serializers
	bufferstream& operator >> (bufferstream& stream, FileInformationBlock& data)
	{
		//read the FIB base
		data.wIdent = binary_io::read_uint16(stream);
		data.nFib = (FibVersion)binary_io::read_uint16(stream);
		data.unknown0 = binary_io::read_uint16(stream);
		data.lid = binary_io::read_uint16(stream);
		data.pnNext = binary_io::read_int16(stream);

		data.flag1 = binary_io::read_uint16(stream);
		// TODO:
		/*
		data.fDot = Utils.BitmaskToBool((int)flag16, 0x0001);
		data.fGlsy = Utils.BitmaskToBool((int)flag16, 0x0002);
		data.fComplex = Utils.BitmaskToBool((int)flag16, 0x0002);
		data.fHasPic = Utils.BitmaskToBool((int)flag16, 0x0008);
		data.cQuickSaves = (uint16_t)(((int)flag16 & 0x00F0) >> 4);
		data.fEncrypted = Utils.BitmaskToBool((int)flag16, 0x0100);
		data.fWhichTblStm = Utils.BitmaskToBool((int)flag16, 0x0200);
		data.fReadOnlyRecommended = Utils.BitmaskToBool((int)flag16, 0x0400);
		data.fWriteReservation = Utils.BitmaskToBool((int)flag16, 0x0800);
		data.fExtChar = Utils.BitmaskToBool((int)flag16, 0x1000);
		data.fLoadOverwrite = Utils.BitmaskToBool((int)flag16, 0x2000);
		data.fFarEast = Utils.BitmaskToBool((int)flag16, 0x4000);
		data.fCrypto = Utils.BitmaskToBool((int)flag16, 0x8000);
		*/
		data.nFibBack = binary_io::read_uint16(stream);
		data.lKey = binary_io::read_int32(stream);
		data.envr = binary_io::read_uint8(stream);

		data.flag2 = binary_io::read_uint8(stream);
		// TODO:
		/*
		data.fMac = Utils.BitmaskToBool((int)flag8, 0x01);
		data.fEmptySpecial = Utils.BitmaskToBool((int)flag8, 0x02);
		data.fLoadOverridePage = Utils.BitmaskToBool((int)flag8, 0x04);
		data.fFutureSavedUndo = Utils.BitmaskToBool((int)flag8, 0x08);
		data.fWord97Saved = Utils.BitmaskToBool((int)flag8, 0x10);
		*/

		data.unknown1 = binary_io::read(stream, 4);
		data.fcMin = binary_io::read_int32(stream);
		data.fcMac = binary_io::read_int32(stream);

		data.csw = binary_io::read_uint16(stream);

		//read the RgW97
		data.unknown2 = binary_io::read(stream, 26);
		data.lidFE = binary_io::read_int16(stream);

		// others
		data.cslw = binary_io::read_uint16(stream);

		//read the RgLW97
		data.cbMac = binary_io::read_int32(stream);
		data.unknown3 = binary_io::read(stream, 8);
		data.ccpText = binary_io::read_int32(stream);
		data.ccpFtn = binary_io::read_int32(stream);
		data.ccpHdr = binary_io::read_int32(stream);
		data.unknown4 = binary_io::read(stream, 4);
		data.ccpAtn = binary_io::read_int32(stream);
		data.ccpEdn = binary_io::read_int32(stream);
		data.ccpTxbx = binary_io::read_int32(stream);
		data.ccpHdrTxbx = binary_io::read_int32(stream);
		data.unknown5 = binary_io::read(stream, 44);

		data.cbRgFcLcb = binary_io::read_uint16(stream);

		if (data.nFib >= FibVersion::Fib1997)
		{
			//Read the FibRgFcLcb97
			data.fcStshfOrig = binary_io::read_uint32(stream);
			data.lcbStshfOrig = binary_io::read_uint32(stream);
			data.fcStshf = binary_io::read_uint32(stream);
			data.lcbStshf = binary_io::read_uint32(stream);
			data.fcPlcffndRef = binary_io::read_uint32(stream);
			data.lcbPlcffndRef = binary_io::read_uint32(stream);
			data.fcPlcffndTxt = binary_io::read_uint32(stream);
			data.lcbPlcffndTxt = binary_io::read_uint32(stream);
			data.fcPlcfandRef = binary_io::read_uint32(stream);
			data.lcbPlcfandRef = binary_io::read_uint32(stream);
			data.fcPlcfandTxt = binary_io::read_uint32(stream);
			data.lcbPlcfandTxt = binary_io::read_uint32(stream);
			data.fcPlcfSed = binary_io::read_uint32(stream);
			data.lcbPlcfSed = binary_io::read_uint32(stream);
			data.fcPlcPad = binary_io::read_uint32(stream);
			data.lcbPlcPad = binary_io::read_uint32(stream);
			data.fcPlcfPhe = binary_io::read_uint32(stream);
			data.lcbPlcfPhe = binary_io::read_uint32(stream);
			data.fcSttbfGlsy = binary_io::read_uint32(stream);
			data.lcbSttbfGlsy = binary_io::read_uint32(stream);
			data.fcPlcfGlsy = binary_io::read_uint32(stream);
			data.lcbPlcfGlsy = binary_io::read_uint32(stream);
			data.fcPlcfHdd = binary_io::read_uint32(stream);
			data.lcbPlcfHdd = binary_io::read_uint32(stream);
			data.fcPlcfBteChpx = binary_io::read_uint32(stream);
			data.lcbPlcfBteChpx = binary_io::read_uint32(stream);
			data.fcPlcfBtePapx = binary_io::read_uint32(stream);
			data.lcbPlcfBtePapx = binary_io::read_uint32(stream);
			data.fcPlcfSea = binary_io::read_uint32(stream);
			data.lcbPlcfSea = binary_io::read_uint32(stream);
			data.fcSttbfFfn = binary_io::read_uint32(stream);
			data.lcbSttbfFfn = binary_io::read_uint32(stream);
			data.fcPlcfFldMom = binary_io::read_uint32(stream);
			data.lcbPlcfFldMom = binary_io::read_uint32(stream);
			data.fcPlcfFldHdr = binary_io::read_uint32(stream);
			data.lcbPlcfFldHdr = binary_io::read_uint32(stream);
			data.fcPlcfFldFtn = binary_io::read_uint32(stream);
			data.lcbPlcfFldFtn = binary_io::read_uint32(stream);
			data.fcPlcfFldAtn = binary_io::read_uint32(stream);
			data.lcbPlcfFldAtn = binary_io::read_uint32(stream);
			data.fcPlcfFldMcr = binary_io::read_uint32(stream);
			data.lcbPlcfFldMcr = binary_io::read_uint32(stream);
			data.fcSttbfBkmk = binary_io::read_uint32(stream);
			data.lcbSttbfBkmk = binary_io::read_uint32(stream);
			data.fcPlcfBkf = binary_io::read_uint32(stream);
			data.lcbPlcfBkf = binary_io::read_uint32(stream);
			data.fcPlcfBkl = binary_io::read_uint32(stream);
			data.lcbPlcfBkl = binary_io::read_uint32(stream);
			data.fcCmds = binary_io::read_uint32(stream);
			data.lcbCmds = binary_io::read_uint32(stream);
			data.unknown6 = binary_io::read_uint32(stream);
			data.unknown7 = binary_io::read_uint32(stream);
			data.fcSttbfMcr = binary_io::read_uint32(stream);
			data.lcbSttbfMcr = binary_io::read_uint32(stream);
			data.fcPrDrvr = binary_io::read_uint32(stream);
			data.lcbPrDrvr = binary_io::read_uint32(stream);
			data.fcPrEnvPort = binary_io::read_uint32(stream);
			data.lcbPrEnvPort = binary_io::read_uint32(stream);
			data.fcPrEnvLand = binary_io::read_uint32(stream);
			data.lcbPrEnvLand = binary_io::read_uint32(stream);
			data.fcWss = binary_io::read_uint32(stream);
			data.lcbWss = binary_io::read_uint32(stream);
			data.fcDop = binary_io::read_uint32(stream);
			data.lcbDop = binary_io::read_uint32(stream);
			data.fcSttbfAssoc = binary_io::read_uint32(stream);
			data.lcbSttbfAssoc = binary_io::read_uint32(stream);
			data.fcClx = binary_io::read_uint32(stream);
			data.lcbClx = binary_io::read_uint32(stream);
			data.fcPlcfPgdFtn = binary_io::read_uint32(stream);
			data.lcbPlcfPgdFtn = binary_io::read_uint32(stream);
			data.fcAutosaveSource = binary_io::read_uint32(stream);
			data.lcbAutosaveSource = binary_io::read_uint32(stream);
			data.fcGrpXstAtnOwners = binary_io::read_uint32(stream);
			data.lcbGrpXstAtnOwners = binary_io::read_uint32(stream);
			data.fcSttbfAtnBkmk = binary_io::read_uint32(stream);
			data.lcbSttbfAtnBkmk = binary_io::read_uint32(stream);
			data.unknown8 = binary_io::read_uint32(stream);
			data.unknown9 = binary_io::read_uint32(stream);
			data.unknown10 = binary_io::read_uint32(stream);
			data.unknown11 = binary_io::read_uint32(stream);
			data.fcPlcSpaMom = binary_io::read_uint32(stream);
			data.lcbPlcSpaMom = binary_io::read_uint32(stream);
			data.fcPlcSpaHdr = binary_io::read_uint32(stream);
			data.lcbPlcSpaHdr = binary_io::read_uint32(stream);
			data.fcPlcfAtnBkf = binary_io::read_uint32(stream);
			data.lcbPlcfAtnBkf = binary_io::read_uint32(stream);
			data.fcPlcfAtnBkl = binary_io::read_uint32(stream);
			data.lcbPlcfAtnBkl = binary_io::read_uint32(stream);
			data.fcPms = binary_io::read_uint32(stream);
			data.lcbPms = binary_io::read_uint32(stream);
			data.fcFormFldSttbs = binary_io::read_uint32(stream);
			data.lcbFormFldSttbs = binary_io::read_uint32(stream);
			data.fcPlcfendRef = binary_io::read_uint32(stream);
			data.lcbPlcfendRef = binary_io::read_uint32(stream);
			data.fcPlcfendTxt = binary_io::read_uint32(stream);
			data.lcbPlcfendTxt = binary_io::read_uint32(stream);
			data.fcPlcfFldEdn = binary_io::read_uint32(stream);
			data.lcbPlcfFldEdn = binary_io::read_uint32(stream);
			data.unknown12 = binary_io::read_uint32(stream);
			data.unknown13 = binary_io::read_uint32(stream);
			data.fcDggInfo = binary_io::read_uint32(stream);
			data.lcbDggInfo = binary_io::read_uint32(stream);
			data.fcSttbfRMark = binary_io::read_uint32(stream);
			data.lcbSttbfRMark = binary_io::read_uint32(stream);
			data.fcSttbfCaption = binary_io::read_uint32(stream);
			data.lcbSttbfCaption = binary_io::read_uint32(stream);
			data.fcSttbfAutoCaption = binary_io::read_uint32(stream);
			data.lcbSttbfAutoCaption = binary_io::read_uint32(stream);
			data.fcPlcfWkb = binary_io::read_uint32(stream);
			data.lcbPlcfWkb = binary_io::read_uint32(stream);
			data.fcPlcfSpl = binary_io::read_uint32(stream);
			data.lcbPlcfSpl = binary_io::read_uint32(stream);
			data.fcPlcftxbxTxt = binary_io::read_uint32(stream);
			data.lcbPlcftxbxTxt = binary_io::read_uint32(stream);
			data.fcPlcfFldTxbx = binary_io::read_uint32(stream);
			data.lcbPlcfFldTxbx = binary_io::read_uint32(stream);
			data.fcPlcfHdrtxbxTxt = binary_io::read_uint32(stream);
			data.lcbPlcfHdrtxbxTxt = binary_io::read_uint32(stream);
			data.fcPlcffldHdrTxbx = binary_io::read_uint32(stream);
			data.lcbPlcffldHdrTxbx = binary_io::read_uint32(stream);
			data.fcStwUser = binary_io::read_uint32(stream);
			data.lcbStwUser = binary_io::read_uint32(stream);
			data.fcSttbTtmbd = binary_io::read_uint32(stream);
			data.lcbSttbTtmbd = binary_io::read_uint32(stream);
			data.fcCookieData = binary_io::read_uint32(stream);
			data.lcbCookieData = binary_io::read_uint32(stream);
			data.fcPgdMotherOldOld = binary_io::read_uint32(stream);
			data.lcbPgdMotherOldOld = binary_io::read_uint32(stream);
			data.fcBkdMotherOldOld = binary_io::read_uint32(stream);
			data.lcbBkdMotherOldOld = binary_io::read_uint32(stream);
			data.fcPgdFtnOldOld = binary_io::read_uint32(stream);
			data.lcbPgdFtnOldOld = binary_io::read_uint32(stream);
			data.fcBkdFtnOldOld = binary_io::read_uint32(stream);
			data.lcbBkdFtnOldOld = binary_io::read_uint32(stream);
			data.fcPgdEdnOldOld = binary_io::read_uint32(stream);
			data.lcbPgdEdnOldOld = binary_io::read_uint32(stream);
			data.fcBkdEdnOldOld = binary_io::read_uint32(stream);
			data.lcbBkdEdnOldOld = binary_io::read_uint32(stream);
			data.fcSttbfIntlFld = binary_io::read_uint32(stream);
			data.lcbSttbfIntlFld = binary_io::read_uint32(stream);
			data.fcRouteSlip = binary_io::read_uint32(stream);
			data.lcbRouteSlip = binary_io::read_uint32(stream);
			data.fcSttbSavedBy = binary_io::read_uint32(stream);
			data.lcbSttbSavedBy = binary_io::read_uint32(stream);
			data.fcSttbFnm = binary_io::read_uint32(stream);
			data.lcbSttbFnm = binary_io::read_uint32(stream);
			data.fcPlfLst = binary_io::read_uint32(stream);
			data.lcbPlfLst = binary_io::read_uint32(stream);
			data.fcPlfLfo = binary_io::read_uint32(stream);
			data.lcbPlfLfo = binary_io::read_uint32(stream);
			data.fcPlcfTxbxBkd = binary_io::read_uint32(stream);
			data.lcbPlcfTxbxBkd = binary_io::read_uint32(stream);
			data.fcPlcfTxbxHdrBkd = binary_io::read_uint32(stream);
			data.lcbPlcfTxbxHdrBkd = binary_io::read_uint32(stream);
			data.fcDocUndoWord9 = binary_io::read_uint32(stream);
			data.lcbDocUndoWord9 = binary_io::read_uint32(stream);
			data.fcRgbUse = binary_io::read_uint32(stream);
			data.lcbRgbUse = binary_io::read_uint32(stream);
			data.fcUsp = binary_io::read_uint32(stream);
			data.lcbUsp = binary_io::read_uint32(stream);
			data.fcUskf = binary_io::read_uint32(stream);
			data.lcbUskf = binary_io::read_uint32(stream);
			data.fcPlcupcRgbUse = binary_io::read_uint32(stream);
			data.lcbPlcupcRgbUse = binary_io::read_uint32(stream);
			data.fcPlcupcUsp = binary_io::read_uint32(stream);
			data.lcbPlcupcUsp = binary_io::read_uint32(stream);
			data.fcSttbGlsyStyle = binary_io::read_uint32(stream);
			data.lcbSttbGlsyStyle = binary_io::read_uint32(stream);
			data.fcPlgosl = binary_io::read_uint32(stream);
			data.lcbPlgosl = binary_io::read_uint32(stream);
			data.fcPlcocx = binary_io::read_uint32(stream);
			data.lcbPlcocx = binary_io::read_uint32(stream);
			data.fcPlcfBteLvc = binary_io::read_uint32(stream);
			data.lcbPlcfBteLvc = binary_io::read_uint32(stream);
			data.dwLowDateTime = binary_io::read_uint32(stream);
			data.dwHighDateTime = binary_io::read_uint32(stream);
			data.fcPlcfLvcPre10 = binary_io::read_uint32(stream);
			data.lcbPlcfLvcPre10 = binary_io::read_uint32(stream);
			data.fcPlcfAsumy = binary_io::read_uint32(stream);
			data.lcbPlcfAsumy = binary_io::read_uint32(stream);
			data.fcPlcfGram = binary_io::read_uint32(stream);
			data.lcbPlcfGram = binary_io::read_uint32(stream);
			data.fcSttbListNames = binary_io::read_uint32(stream);
			data.lcbSttbListNames = binary_io::read_uint32(stream);
			data.fcSttbfUssr = binary_io::read_uint32(stream);
			data.lcbSttbfUssr = binary_io::read_uint32(stream);
		}
		if (data.nFib >= FibVersion::Fib2000)
		{
			//Read also the FibRgFcLcb2000
			data.fcPlcfTch = binary_io::read_uint32(stream);
			data.lcbPlcfTch = binary_io::read_uint32(stream);
			data.fcRmdThreading = binary_io::read_uint32(stream);
			data.lcbRmdThreading = binary_io::read_uint32(stream);
			data.fcMid = binary_io::read_uint32(stream);
			data.lcbMid = binary_io::read_uint32(stream);
			data.fcSttbRgtplc = binary_io::read_uint32(stream);
			data.lcbSttbRgtplc = binary_io::read_uint32(stream);
			data.fcMsoEnvelope = binary_io::read_uint32(stream);
			data.lcbMsoEnvelope = binary_io::read_uint32(stream);
			data.fcPlcfLad = binary_io::read_uint32(stream);
			data.lcbPlcfLad = binary_io::read_uint32(stream);
			data.fcRgDofr = binary_io::read_uint32(stream);
			data.lcbRgDofr = binary_io::read_uint32(stream);
			data.fcPlcosl = binary_io::read_uint32(stream);
			data.lcbPlcosl = binary_io::read_uint32(stream);
			data.fcPlcfCookieOld = binary_io::read_uint32(stream);
			data.lcbPlcfCookieOld = binary_io::read_uint32(stream);
			data.fcPgdMotherOld = binary_io::read_uint32(stream);
			data.lcbPgdMotherOld = binary_io::read_uint32(stream);
			data.fcBkdMotherOld = binary_io::read_uint32(stream);
			data.lcbBkdMotherOld = binary_io::read_uint32(stream);
			data.fcPgdFtnOld = binary_io::read_uint32(stream);
			data.lcbPgdFtnOld = binary_io::read_uint32(stream);
			data.fcBkdFtnOld = binary_io::read_uint32(stream);
			data.lcbBkdFtnOld = binary_io::read_uint32(stream);
			data.fcPgdEdnOld = binary_io::read_uint32(stream);
			data.lcbPgdEdnOld = binary_io::read_uint32(stream);
			data.fcBkdEdnOld = binary_io::read_uint32(stream);
			data.lcbBkdEdnOld = binary_io::read_uint32(stream);
		}
		if (data.nFib >= FibVersion::Fib2002)
		{
			//Read also the fibRgFcLcb2002
			data.unknown14 = binary_io::read_uint32(stream);
			data.unknown15 = binary_io::read_uint32(stream);
			data.fcPlcfPgp = binary_io::read_uint32(stream);
			data.lcbPlcfPgp = binary_io::read_uint32(stream);
			data.fcPlcfuim = binary_io::read_uint32(stream);
			data.lcbPlcfuim = binary_io::read_uint32(stream);
			data.fcPlfguidUim = binary_io::read_uint32(stream);
			data.lcbPlfguidUim = binary_io::read_uint32(stream);
			data.fcAtrdExtra = binary_io::read_uint32(stream);
			data.lcbAtrdExtra = binary_io::read_uint32(stream);
			data.fcPlrsid = binary_io::read_uint32(stream);
			data.lcbPlrsid = binary_io::read_uint32(stream);
			data.fcSttbfBkmkFactoid = binary_io::read_uint32(stream);
			data.lcbSttbfBkmkFactoid = binary_io::read_uint32(stream);
			data.fcPlcfBkfFactoid = binary_io::read_uint32(stream);
			data.lcbPlcfBkfFactoid = binary_io::read_uint32(stream);
			data.fcPlcfcookie = binary_io::read_uint32(stream);
			data.lcbPlcfcookie = binary_io::read_uint32(stream);
			data.fcPlcfBklFactoid = binary_io::read_uint32(stream);
			data.lcbPlcfBklFactoid = binary_io::read_uint32(stream);
			data.fcFactoidData = binary_io::read_uint32(stream);
			data.lcbFactoidData = binary_io::read_uint32(stream);
			data.fcDocUndo = binary_io::read_uint32(stream);
			data.lcbDocUndo = binary_io::read_uint32(stream);
			data.fcSttbfBkmkFcc = binary_io::read_uint32(stream);
			data.lcbSttbfBkmkFcc = binary_io::read_uint32(stream);
			data.fcPlcfBkfFcc = binary_io::read_uint32(stream);
			data.lcbPlcfBkfFcc = binary_io::read_uint32(stream);
			data.fcPlcfBklFcc = binary_io::read_uint32(stream);
			data.lcbPlcfBklFcc = binary_io::read_uint32(stream);
			data.fcSttbfbkmkBPRepairs = binary_io::read_uint32(stream);
			data.lcbSttbfbkmkBPRepairs = binary_io::read_uint32(stream);
			data.fcPlcfbkfBPRepairs = binary_io::read_uint32(stream);
			data.lcbPlcfbkfBPRepairs = binary_io::read_uint32(stream);
			data.fcPlcfbklBPRepairs = binary_io::read_uint32(stream);
			data.lcbPlcfbklBPRepairs = binary_io::read_uint32(stream);
			data.fcPmsNew = binary_io::read_uint32(stream);
			data.lcbPmsNew = binary_io::read_uint32(stream);
			data.fcODSO = binary_io::read_uint32(stream);
			data.lcbODSO = binary_io::read_uint32(stream);
			data.fcPlcfpmiOldXP = binary_io::read_uint32(stream);
			data.lcbPlcfpmiOldXP = binary_io::read_uint32(stream);
			data.fcPlcfpmiNewXP = binary_io::read_uint32(stream);
			data.lcbPlcfpmiNewXP = binary_io::read_uint32(stream);
			data.fcPlcfpmiMixedXP = binary_io::read_uint32(stream);
			data.lcbPlcfpmiMixedXP = binary_io::read_uint32(stream);
			data.unknown17 = binary_io::read_uint32(stream);
			data.unknown18 = binary_io::read_uint32(stream);
			data.fcPlcffactoid = binary_io::read_uint32(stream);
			data.lcbPlcffactoid = binary_io::read_uint32(stream);
			data.fcPlcflvcOldXP = binary_io::read_uint32(stream);
			data.lcbPlcflvcOldXP = binary_io::read_uint32(stream);
			data.fcPlcflvcNewXP = binary_io::read_uint32(stream);
			data.lcbPlcflvcNewXP = binary_io::read_uint32(stream);
			data.fcPlcflvcMixedXP = binary_io::read_uint32(stream);
			data.lcbPlcflvcMixedXP = binary_io::read_uint32(stream);
		}
		if (data.nFib >= FibVersion::Fib2003)
		{
			//Read also the fibRgFcLcb2003
			data.fcHplxsdr = binary_io::read_uint32(stream);
			data.lcbHplxsdr = binary_io::read_uint32(stream);
			data.fcSttbfBkmkSdt = binary_io::read_uint32(stream);
			data.lcbSttbfBkmkSdt = binary_io::read_uint32(stream);
			data.fcPlcfBkfSdt = binary_io::read_uint32(stream);
			data.lcbPlcfBkfSdt = binary_io::read_uint32(stream);
			data.fcPlcfBklSdt = binary_io::read_uint32(stream);
			data.lcbPlcfBklSdt = binary_io::read_uint32(stream);
			data.fcCustomXForm = binary_io::read_uint32(stream);
			data.lcbCustomXForm = binary_io::read_uint32(stream);
			data.fcSttbfBkmkProt = binary_io::read_uint32(stream);
			data.lcbSttbfBkmkProt = binary_io::read_uint32(stream);
			data.fcPlcfBkfProt = binary_io::read_uint32(stream);
			data.lcbPlcfBkfProt = binary_io::read_uint32(stream);
			data.fcPlcfBklProt = binary_io::read_uint32(stream);
			data.lcbPlcfBklProt = binary_io::read_uint32(stream);
			data.fcSttbProtUser = binary_io::read_uint32(stream);
			data.lcbSttbProtUser = binary_io::read_uint32(stream);
			data.unknown19 = binary_io::read_uint32(stream);
			data.unknown20 = binary_io::read_uint32(stream);
			data.fcPlcfpmiOld = binary_io::read_uint32(stream);
			data.lcbPlcfpmiOld = binary_io::read_uint32(stream);
			data.fcPlcfpmiOldInline = binary_io::read_uint32(stream);
			data.lcbPlcfpmiOldInline = binary_io::read_uint32(stream);
			data.fcPlcfpmiNew = binary_io::read_uint32(stream);
			data.lcbPlcfpmiNew = binary_io::read_uint32(stream);
			data.fcPlcfpmiNewInline = binary_io::read_uint32(stream);
			data.lcbPlcfpmiNewInline = binary_io::read_uint32(stream);
			data.fcPlcflvcOld = binary_io::read_uint32(stream);
			data.lcbPlcflvcOld = binary_io::read_uint32(stream);
			data.fcPlcflvcOldInline = binary_io::read_uint32(stream);
			data.lcbPlcflvcOldInline = binary_io::read_uint32(stream);
			data.fcPlcflvcNew = binary_io::read_uint32(stream);
			data.lcbPlcflvcNew = binary_io::read_uint32(stream);
			data.fcPlcflvcNewInline = binary_io::read_uint32(stream);
			data.lcbPlcflvcNewInline = binary_io::read_uint32(stream);
			data.fcPgdMother = binary_io::read_uint32(stream);
			data.lcbPgdMother = binary_io::read_uint32(stream);
			data.fcBkdMother = binary_io::read_uint32(stream);
			data.lcbBkdMother = binary_io::read_uint32(stream);
			data.fcAfdMother = binary_io::read_uint32(stream);
			data.lcbAfdMother = binary_io::read_uint32(stream);
			data.fcPgdFtn = binary_io::read_uint32(stream);
			data.lcbPgdFtn = binary_io::read_uint32(stream);
			data.fcBkdFtn = binary_io::read_uint32(stream);
			data.lcbBkdFtn = binary_io::read_uint32(stream);
			data.fcAfdFtn = binary_io::read_uint32(stream);
			data.lcbAfdFtn = binary_io::read_uint32(stream);
			data.fcPgdEdn = binary_io::read_uint32(stream);
			data.lcbPgdEdn = binary_io::read_uint32(stream);
			data.fcBkdEdn = binary_io::read_uint32(stream);
			data.lcbBkdEdn = binary_io::read_uint32(stream);
			data.fcAfdEdn = binary_io::read_uint32(stream);
			data.lcbAfdEdn = binary_io::read_uint32(stream);
			data.fcAfd = binary_io::read_uint32(stream);
			data.lcbAfd = binary_io::read_uint32(stream);
		}
		if (data.nFib >= FibVersion::Fib2007)
		{
			//Read also the fibRgFcLcb2007
			data.fcPlcfmthd = binary_io::read_uint32(stream);
			data.lcbPlcfmthd = binary_io::read_uint32(stream);
			data.fcSttbfBkmkMoveFrom = binary_io::read_uint32(stream);
			data.lcbSttbfBkmkMoveFrom = binary_io::read_uint32(stream);
			data.fcPlcfBkfMoveFrom = binary_io::read_uint32(stream);
			data.lcbPlcfBkfMoveFrom = binary_io::read_uint32(stream);
			data.fcPlcfBklMoveFrom = binary_io::read_uint32(stream);
			data.lcbPlcfBklMoveFrom = binary_io::read_uint32(stream);
			data.fcSttbfBkmkMoveTo = binary_io::read_uint32(stream);
			data.lcbSttbfBkmkMoveTo = binary_io::read_uint32(stream);
			data.fcPlcfBkfMoveTo = binary_io::read_uint32(stream);
			data.lcbPlcfBkfMoveTo = binary_io::read_uint32(stream);
			data.fcPlcfBklMoveTo = binary_io::read_uint32(stream);
			data.lcbPlcfBklMoveTo = binary_io::read_uint32(stream);
			data.unknown21 = binary_io::read_uint32(stream);
			data.unknown22 = binary_io::read_uint32(stream);
			data.unknown23 = binary_io::read_uint32(stream);
			data.unknown24 = binary_io::read_uint32(stream);
			data.unknown25 = binary_io::read_uint32(stream);
			data.unknown26 = binary_io::read_uint32(stream);
			data.fcSttbfBkmkArto = binary_io::read_uint32(stream);
			data.lcbSttbfBkmkArto = binary_io::read_uint32(stream);
			data.fcPlcfBkfArto = binary_io::read_uint32(stream);
			data.lcbPlcfBkfArto = binary_io::read_uint32(stream);
			data.fcPlcfBklArto = binary_io::read_uint32(stream);
			data.lcbPlcfBklArto = binary_io::read_uint32(stream);
			data.fcArtoData = binary_io::read_uint32(stream);
			data.lcbArtoData = binary_io::read_uint32(stream);
			data.unknown27 = binary_io::read_uint32(stream);
			data.unknown28 = binary_io::read_uint32(stream);
			data.unknown29 = binary_io::read_uint32(stream);
			data.unknown30 = binary_io::read_uint32(stream);
			data.unknown31 = binary_io::read_uint32(stream);
			data.unknown32 = binary_io::read_uint32(stream);
			data.fcOssTheme = binary_io::read_uint32(stream);
			data.lcbOssTheme = binary_io::read_uint32(stream);
			data.fcColorSchemeMapping = binary_io::read_uint32(stream);
			data.lcbColorSchemeMapping = binary_io::read_uint32(stream);
		}

		data.cswNew = binary_io::read_uint16(stream);

		if (data.cswNew != 0)
		{
			//Read the FibRgCswNew
			data.nFibNew = (FibVersion)binary_io::read_uint16(stream);
			data.cQuickSavesNew = binary_io::read_uint16(stream);
		}
		return stream;
	}

	bufferstream& operator << (bufferstream& stream, const FileInformationBlock& data)
	{
		//read the FIB base
		binary_io::write_uint16(stream, data.wIdent);
		binary_io::write_uint16(stream, data.nFib);
		binary_io::write_uint16(stream, data.unknown0);
		binary_io::write_uint16(stream, data.lid);
		binary_io::write_int16(stream, data.pnNext);

		binary_io::write_uint16(stream, data.flag1.to_ulong());

		binary_io::write_uint16(stream, data.nFibBack);
		binary_io::write_int32(stream, data.lKey);
		binary_io::write_uint8(stream, data.envr);

		binary_io::write_uint8(stream, data.flag2.to_ulong());

		binary_io::write(stream, data.unknown1);
		binary_io::write_int32(stream, data.fcMin);
		binary_io::write_int32(stream, data.fcMac);

		binary_io::write_uint16(stream, data.csw);

		//read the RgW97
		binary_io::write(stream, data.unknown2);
		binary_io::write_int16(stream, data.lidFE);

		// others
		binary_io::write_uint16(stream, data.cslw);

		//read the RgLW97
		binary_io::write_int32(stream, data.cbMac);
		binary_io::write(stream, data.unknown3);
		binary_io::write_int32(stream, data.ccpText);
		binary_io::write_int32(stream, data.ccpFtn);
		binary_io::write_int32(stream, data.ccpHdr);
		binary_io::write(stream, data.unknown4);
		binary_io::write_int32(stream, data.ccpAtn);
		binary_io::write_int32(stream, data.ccpEdn);
		binary_io::write_int32(stream, data.ccpTxbx);
		binary_io::write_int32(stream, data.ccpHdrTxbx);
		binary_io::write(stream, data.unknown5);

		binary_io::write_uint16(stream, data.cbRgFcLcb);

		if (data.nFib >= FibVersion::Fib1997)
		{
			//Read the FibRgFcLcb97
			binary_io::write_uint32(stream, data.fcStshfOrig);
			binary_io::write_uint32(stream, data.lcbStshfOrig);
			binary_io::write_uint32(stream, data.fcStshf);
			binary_io::write_uint32(stream, data.lcbStshf);
			binary_io::write_uint32(stream, data.fcPlcffndRef);
			binary_io::write_uint32(stream, data.lcbPlcffndRef);
			binary_io::write_uint32(stream, data.fcPlcffndTxt);
			binary_io::write_uint32(stream, data.lcbPlcffndTxt);
			binary_io::write_uint32(stream, data.fcPlcfandRef);
			binary_io::write_uint32(stream, data.lcbPlcfandRef);
			binary_io::write_uint32(stream, data.fcPlcfandTxt);
			binary_io::write_uint32(stream, data.lcbPlcfandTxt);
			binary_io::write_uint32(stream, data.fcPlcfSed);
			binary_io::write_uint32(stream, data.lcbPlcfSed);
			binary_io::write_uint32(stream, data.fcPlcPad);
			binary_io::write_uint32(stream, data.lcbPlcPad);
			binary_io::write_uint32(stream, data.fcPlcfPhe);
			binary_io::write_uint32(stream, data.lcbPlcfPhe);
			binary_io::write_uint32(stream, data.fcSttbfGlsy);
			binary_io::write_uint32(stream, data.lcbSttbfGlsy);
			binary_io::write_uint32(stream, data.fcPlcfGlsy);
			binary_io::write_uint32(stream, data.lcbPlcfGlsy);
			binary_io::write_uint32(stream, data.fcPlcfHdd);
			binary_io::write_uint32(stream, data.lcbPlcfHdd);
			binary_io::write_uint32(stream, data.fcPlcfBteChpx);
			binary_io::write_uint32(stream, data.lcbPlcfBteChpx);
			binary_io::write_uint32(stream, data.fcPlcfBtePapx);
			binary_io::write_uint32(stream, data.lcbPlcfBtePapx);
			binary_io::write_uint32(stream, data.fcPlcfSea);
			binary_io::write_uint32(stream, data.lcbPlcfSea);
			binary_io::write_uint32(stream, data.fcSttbfFfn);
			binary_io::write_uint32(stream, data.lcbSttbfFfn);
			binary_io::write_uint32(stream, data.fcPlcfFldMom);
			binary_io::write_uint32(stream, data.lcbPlcfFldMom);
			binary_io::write_uint32(stream, data.fcPlcfFldHdr);
			binary_io::write_uint32(stream, data.lcbPlcfFldHdr);
			binary_io::write_uint32(stream, data.fcPlcfFldFtn);
			binary_io::write_uint32(stream, data.lcbPlcfFldFtn);
			binary_io::write_uint32(stream, data.fcPlcfFldAtn);
			binary_io::write_uint32(stream, data.lcbPlcfFldAtn);
			binary_io::write_uint32(stream, data.fcPlcfFldMcr);
			binary_io::write_uint32(stream, data.lcbPlcfFldMcr);
			binary_io::write_uint32(stream, data.fcSttbfBkmk);
			binary_io::write_uint32(stream, data.lcbSttbfBkmk);
			binary_io::write_uint32(stream, data.fcPlcfBkf);
			binary_io::write_uint32(stream, data.lcbPlcfBkf);
			binary_io::write_uint32(stream, data.fcPlcfBkl);
			binary_io::write_uint32(stream, data.lcbPlcfBkl);
			binary_io::write_uint32(stream, data.fcCmds);
			binary_io::write_uint32(stream, data.lcbCmds);
			binary_io::write_uint32(stream, data.unknown6);
			binary_io::write_uint32(stream, data.unknown7);
			binary_io::write_uint32(stream, data.fcSttbfMcr);
			binary_io::write_uint32(stream, data.lcbSttbfMcr);
			binary_io::write_uint32(stream, data.fcPrDrvr);
			binary_io::write_uint32(stream, data.lcbPrDrvr);
			binary_io::write_uint32(stream, data.fcPrEnvPort);
			binary_io::write_uint32(stream, data.lcbPrEnvPort);
			binary_io::write_uint32(stream, data.fcPrEnvLand);
			binary_io::write_uint32(stream, data.lcbPrEnvLand);
			binary_io::write_uint32(stream, data.fcWss);
			binary_io::write_uint32(stream, data.lcbWss);
			binary_io::write_uint32(stream, data.fcDop);
			binary_io::write_uint32(stream, data.lcbDop);
			binary_io::write_uint32(stream, data.fcSttbfAssoc);
			binary_io::write_uint32(stream, data.lcbSttbfAssoc);
			binary_io::write_uint32(stream, data.fcClx);
			binary_io::write_uint32(stream, data.lcbClx);
			binary_io::write_uint32(stream, data.fcPlcfPgdFtn);
			binary_io::write_uint32(stream, data.lcbPlcfPgdFtn);
			binary_io::write_uint32(stream, data.fcAutosaveSource);
			binary_io::write_uint32(stream, data.lcbAutosaveSource);
			binary_io::write_uint32(stream, data.fcGrpXstAtnOwners);
			binary_io::write_uint32(stream, data.lcbGrpXstAtnOwners);
			binary_io::write_uint32(stream, data.fcSttbfAtnBkmk);
			binary_io::write_uint32(stream, data.lcbSttbfAtnBkmk);
			binary_io::write_uint32(stream, data.unknown8);
			binary_io::write_uint32(stream, data.unknown9);
			binary_io::write_uint32(stream, data.unknown10);
			binary_io::write_uint32(stream, data.unknown11);
			binary_io::write_uint32(stream, data.fcPlcSpaMom);
			binary_io::write_uint32(stream, data.lcbPlcSpaMom);
			binary_io::write_uint32(stream, data.fcPlcSpaHdr);
			binary_io::write_uint32(stream, data.lcbPlcSpaHdr);
			binary_io::write_uint32(stream, data.fcPlcfAtnBkf);
			binary_io::write_uint32(stream, data.lcbPlcfAtnBkf);
			binary_io::write_uint32(stream, data.fcPlcfAtnBkl);
			binary_io::write_uint32(stream, data.lcbPlcfAtnBkl);
			binary_io::write_uint32(stream, data.fcPms);
			binary_io::write_uint32(stream, data.lcbPms);
			binary_io::write_uint32(stream, data.fcFormFldSttbs);
			binary_io::write_uint32(stream, data.lcbFormFldSttbs);
			binary_io::write_uint32(stream, data.fcPlcfendRef);
			binary_io::write_uint32(stream, data.lcbPlcfendRef);
			binary_io::write_uint32(stream, data.fcPlcfendTxt);
			binary_io::write_uint32(stream, data.lcbPlcfendTxt);
			binary_io::write_uint32(stream, data.fcPlcfFldEdn);
			binary_io::write_uint32(stream, data.lcbPlcfFldEdn);
			binary_io::write_uint32(stream, data.unknown12);
			binary_io::write_uint32(stream, data.unknown13);
			binary_io::write_uint32(stream, data.fcDggInfo);
			binary_io::write_uint32(stream, data.lcbDggInfo);
			binary_io::write_uint32(stream, data.fcSttbfRMark);
			binary_io::write_uint32(stream, data.lcbSttbfRMark);
			binary_io::write_uint32(stream, data.fcSttbfCaption);
			binary_io::write_uint32(stream, data.lcbSttbfCaption);
			binary_io::write_uint32(stream, data.fcSttbfAutoCaption);
			binary_io::write_uint32(stream, data.lcbSttbfAutoCaption);
			binary_io::write_uint32(stream, data.fcPlcfWkb);
			binary_io::write_uint32(stream, data.lcbPlcfWkb);
			binary_io::write_uint32(stream, data.fcPlcfSpl);
			binary_io::write_uint32(stream, data.lcbPlcfSpl);
			binary_io::write_uint32(stream, data.fcPlcftxbxTxt);
			binary_io::write_uint32(stream, data.lcbPlcftxbxTxt);
			binary_io::write_uint32(stream, data.fcPlcfFldTxbx);
			binary_io::write_uint32(stream, data.lcbPlcfFldTxbx);
			binary_io::write_uint32(stream, data.fcPlcfHdrtxbxTxt);
			binary_io::write_uint32(stream, data.lcbPlcfHdrtxbxTxt);
			binary_io::write_uint32(stream, data.fcPlcffldHdrTxbx);
			binary_io::write_uint32(stream, data.lcbPlcffldHdrTxbx);
			binary_io::write_uint32(stream, data.fcStwUser);
			binary_io::write_uint32(stream, data.lcbStwUser);
			binary_io::write_uint32(stream, data.fcSttbTtmbd);
			binary_io::write_uint32(stream, data.lcbSttbTtmbd);
			binary_io::write_uint32(stream, data.fcCookieData);
			binary_io::write_uint32(stream, data.lcbCookieData);
			binary_io::write_uint32(stream, data.fcPgdMotherOldOld);
			binary_io::write_uint32(stream, data.lcbPgdMotherOldOld);
			binary_io::write_uint32(stream, data.fcBkdMotherOldOld);
			binary_io::write_uint32(stream, data.lcbBkdMotherOldOld);
			binary_io::write_uint32(stream, data.fcPgdFtnOldOld);
			binary_io::write_uint32(stream, data.lcbPgdFtnOldOld);
			binary_io::write_uint32(stream, data.fcBkdFtnOldOld);
			binary_io::write_uint32(stream, data.lcbBkdFtnOldOld);
			binary_io::write_uint32(stream, data.fcPgdEdnOldOld);
			binary_io::write_uint32(stream, data.lcbPgdEdnOldOld);
			binary_io::write_uint32(stream, data.fcBkdEdnOldOld);
			binary_io::write_uint32(stream, data.lcbBkdEdnOldOld);
			binary_io::write_uint32(stream, data.fcSttbfIntlFld);
			binary_io::write_uint32(stream, data.lcbSttbfIntlFld);
			binary_io::write_uint32(stream, data.fcRouteSlip);
			binary_io::write_uint32(stream, data.lcbRouteSlip);
			binary_io::write_uint32(stream, data.fcSttbSavedBy);
			binary_io::write_uint32(stream, data.lcbSttbSavedBy);
			binary_io::write_uint32(stream, data.fcSttbFnm);
			binary_io::write_uint32(stream, data.lcbSttbFnm);
			binary_io::write_uint32(stream, data.fcPlfLst);
			binary_io::write_uint32(stream, data.lcbPlfLst);
			binary_io::write_uint32(stream, data.fcPlfLfo);
			binary_io::write_uint32(stream, data.lcbPlfLfo);
			binary_io::write_uint32(stream, data.fcPlcfTxbxBkd);
			binary_io::write_uint32(stream, data.lcbPlcfTxbxBkd);
			binary_io::write_uint32(stream, data.fcPlcfTxbxHdrBkd);
			binary_io::write_uint32(stream, data.lcbPlcfTxbxHdrBkd);
			binary_io::write_uint32(stream, data.fcDocUndoWord9);
			binary_io::write_uint32(stream, data.lcbDocUndoWord9);
			binary_io::write_uint32(stream, data.fcRgbUse);
			binary_io::write_uint32(stream, data.lcbRgbUse);
			binary_io::write_uint32(stream, data.fcUsp);
			binary_io::write_uint32(stream, data.lcbUsp);
			binary_io::write_uint32(stream, data.fcUskf);
			binary_io::write_uint32(stream, data.lcbUskf);
			binary_io::write_uint32(stream, data.fcPlcupcRgbUse);
			binary_io::write_uint32(stream, data.lcbPlcupcRgbUse);
			binary_io::write_uint32(stream, data.fcPlcupcUsp);
			binary_io::write_uint32(stream, data.lcbPlcupcUsp);
			binary_io::write_uint32(stream, data.fcSttbGlsyStyle);
			binary_io::write_uint32(stream, data.lcbSttbGlsyStyle);
			binary_io::write_uint32(stream, data.fcPlgosl);
			binary_io::write_uint32(stream, data.lcbPlgosl);
			binary_io::write_uint32(stream, data.fcPlcocx);
			binary_io::write_uint32(stream, data.lcbPlcocx);
			binary_io::write_uint32(stream, data.fcPlcfBteLvc);
			binary_io::write_uint32(stream, data.lcbPlcfBteLvc);
			binary_io::write_uint32(stream, data.dwLowDateTime);
			binary_io::write_uint32(stream, data.dwHighDateTime);
			binary_io::write_uint32(stream, data.fcPlcfLvcPre10);
			binary_io::write_uint32(stream, data.lcbPlcfLvcPre10);
			binary_io::write_uint32(stream, data.fcPlcfAsumy);
			binary_io::write_uint32(stream, data.lcbPlcfAsumy);
			binary_io::write_uint32(stream, data.fcPlcfGram);
			binary_io::write_uint32(stream, data.lcbPlcfGram);
			binary_io::write_uint32(stream, data.fcSttbListNames);
			binary_io::write_uint32(stream, data.lcbSttbListNames);
			binary_io::write_uint32(stream, data.fcSttbfUssr);
			binary_io::write_uint32(stream, data.lcbSttbfUssr);
		}
		if (data.nFib >= FibVersion::Fib2000)
		{
			//Read also the FibRgFcLcb2000
			binary_io::write_uint32(stream, data.fcPlcfTch);
			binary_io::write_uint32(stream, data.lcbPlcfTch);
			binary_io::write_uint32(stream, data.fcRmdThreading);
			binary_io::write_uint32(stream, data.lcbRmdThreading);
			binary_io::write_uint32(stream, data.fcMid);
			binary_io::write_uint32(stream, data.lcbMid);
			binary_io::write_uint32(stream, data.fcSttbRgtplc);
			binary_io::write_uint32(stream, data.lcbSttbRgtplc);
			binary_io::write_uint32(stream, data.fcMsoEnvelope);
			binary_io::write_uint32(stream, data.lcbMsoEnvelope);
			binary_io::write_uint32(stream, data.fcPlcfLad);
			binary_io::write_uint32(stream, data.lcbPlcfLad);
			binary_io::write_uint32(stream, data.fcRgDofr);
			binary_io::write_uint32(stream, data.lcbRgDofr);
			binary_io::write_uint32(stream, data.fcPlcosl);
			binary_io::write_uint32(stream, data.lcbPlcosl);
			binary_io::write_uint32(stream, data.fcPlcfCookieOld);
			binary_io::write_uint32(stream, data.lcbPlcfCookieOld);
			binary_io::write_uint32(stream, data.fcPgdMotherOld);
			binary_io::write_uint32(stream, data.lcbPgdMotherOld);
			binary_io::write_uint32(stream, data.fcBkdMotherOld);
			binary_io::write_uint32(stream, data.lcbBkdMotherOld);
			binary_io::write_uint32(stream, data.fcPgdFtnOld);
			binary_io::write_uint32(stream, data.lcbPgdFtnOld);
			binary_io::write_uint32(stream, data.fcBkdFtnOld);
			binary_io::write_uint32(stream, data.lcbBkdFtnOld);
			binary_io::write_uint32(stream, data.fcPgdEdnOld);
			binary_io::write_uint32(stream, data.lcbPgdEdnOld);
			binary_io::write_uint32(stream, data.fcBkdEdnOld);
			binary_io::write_uint32(stream, data.lcbBkdEdnOld);
		}
		if (data.nFib >= FibVersion::Fib2002)
		{
			//Read also the fibRgFcLcb2002
			binary_io::write_uint32(stream, data.unknown14);
			binary_io::write_uint32(stream, data.unknown15);
			binary_io::write_uint32(stream, data.fcPlcfPgp);
			binary_io::write_uint32(stream, data.lcbPlcfPgp);
			binary_io::write_uint32(stream, data.fcPlcfuim);
			binary_io::write_uint32(stream, data.lcbPlcfuim);
			binary_io::write_uint32(stream, data.fcPlfguidUim);
			binary_io::write_uint32(stream, data.lcbPlfguidUim);
			binary_io::write_uint32(stream, data.fcAtrdExtra);
			binary_io::write_uint32(stream, data.lcbAtrdExtra);
			binary_io::write_uint32(stream, data.fcPlrsid);
			binary_io::write_uint32(stream, data.lcbPlrsid);
			binary_io::write_uint32(stream, data.fcSttbfBkmkFactoid);
			binary_io::write_uint32(stream, data.lcbSttbfBkmkFactoid);
			binary_io::write_uint32(stream, data.fcPlcfBkfFactoid);
			binary_io::write_uint32(stream, data.lcbPlcfBkfFactoid);
			binary_io::write_uint32(stream, data.fcPlcfcookie);
			binary_io::write_uint32(stream, data.lcbPlcfcookie);
			binary_io::write_uint32(stream, data.fcPlcfBklFactoid);
			binary_io::write_uint32(stream, data.lcbPlcfBklFactoid);
			binary_io::write_uint32(stream, data.fcFactoidData);
			binary_io::write_uint32(stream, data.lcbFactoidData);
			binary_io::write_uint32(stream, data.fcDocUndo);
			binary_io::write_uint32(stream, data.lcbDocUndo);
			binary_io::write_uint32(stream, data.fcSttbfBkmkFcc);
			binary_io::write_uint32(stream, data.lcbSttbfBkmkFcc);
			binary_io::write_uint32(stream, data.fcPlcfBkfFcc);
			binary_io::write_uint32(stream, data.lcbPlcfBkfFcc);
			binary_io::write_uint32(stream, data.fcPlcfBklFcc);
			binary_io::write_uint32(stream, data.lcbPlcfBklFcc);
			binary_io::write_uint32(stream, data.fcSttbfbkmkBPRepairs);
			binary_io::write_uint32(stream, data.lcbSttbfbkmkBPRepairs);
			binary_io::write_uint32(stream, data.fcPlcfbkfBPRepairs);
			binary_io::write_uint32(stream, data.lcbPlcfbkfBPRepairs);
			binary_io::write_uint32(stream, data.fcPlcfbklBPRepairs);
			binary_io::write_uint32(stream, data.lcbPlcfbklBPRepairs);
			binary_io::write_uint32(stream, data.fcPmsNew);
			binary_io::write_uint32(stream, data.lcbPmsNew);
			binary_io::write_uint32(stream, data.fcODSO);
			binary_io::write_uint32(stream, data.lcbODSO);
			binary_io::write_uint32(stream, data.fcPlcfpmiOldXP);
			binary_io::write_uint32(stream, data.lcbPlcfpmiOldXP);
			binary_io::write_uint32(stream, data.fcPlcfpmiNewXP);
			binary_io::write_uint32(stream, data.lcbPlcfpmiNewXP);
			binary_io::write_uint32(stream, data.fcPlcfpmiMixedXP);
			binary_io::write_uint32(stream, data.lcbPlcfpmiMixedXP);
			binary_io::write_uint32(stream, data.unknown17);
			binary_io::write_uint32(stream, data.unknown18);
			binary_io::write_uint32(stream, data.fcPlcffactoid);
			binary_io::write_uint32(stream, data.lcbPlcffactoid);
			binary_io::write_uint32(stream, data.fcPlcflvcOldXP);
			binary_io::write_uint32(stream, data.lcbPlcflvcOldXP);
			binary_io::write_uint32(stream, data.fcPlcflvcNewXP);
			binary_io::write_uint32(stream, data.lcbPlcflvcNewXP);
			binary_io::write_uint32(stream, data.fcPlcflvcMixedXP);
			binary_io::write_uint32(stream, data.lcbPlcflvcMixedXP);
		}
		if (data.nFib >= FibVersion::Fib2003)
		{
			//Read also the fibRgFcLcb2003
			binary_io::write_uint32(stream, data.fcHplxsdr);
			binary_io::write_uint32(stream, data.lcbHplxsdr);
			binary_io::write_uint32(stream, data.fcSttbfBkmkSdt);
			binary_io::write_uint32(stream, data.lcbSttbfBkmkSdt);
			binary_io::write_uint32(stream, data.fcPlcfBkfSdt);
			binary_io::write_uint32(stream, data.lcbPlcfBkfSdt);
			binary_io::write_uint32(stream, data.fcPlcfBklSdt);
			binary_io::write_uint32(stream, data.lcbPlcfBklSdt);
			binary_io::write_uint32(stream, data.fcCustomXForm);
			binary_io::write_uint32(stream, data.lcbCustomXForm);
			binary_io::write_uint32(stream, data.fcSttbfBkmkProt);
			binary_io::write_uint32(stream, data.lcbSttbfBkmkProt);
			binary_io::write_uint32(stream, data.fcPlcfBkfProt);
			binary_io::write_uint32(stream, data.lcbPlcfBkfProt);
			binary_io::write_uint32(stream, data.fcPlcfBklProt);
			binary_io::write_uint32(stream, data.lcbPlcfBklProt);
			binary_io::write_uint32(stream, data.fcSttbProtUser);
			binary_io::write_uint32(stream, data.lcbSttbProtUser);
			binary_io::write_uint32(stream, data.unknown19);
			binary_io::write_uint32(stream, data.unknown20);
			binary_io::write_uint32(stream, data.fcPlcfpmiOld);
			binary_io::write_uint32(stream, data.lcbPlcfpmiOld);
			binary_io::write_uint32(stream, data.fcPlcfpmiOldInline);
			binary_io::write_uint32(stream, data.lcbPlcfpmiOldInline);
			binary_io::write_uint32(stream, data.fcPlcfpmiNew);
			binary_io::write_uint32(stream, data.lcbPlcfpmiNew);
			binary_io::write_uint32(stream, data.fcPlcfpmiNewInline);
			binary_io::write_uint32(stream, data.lcbPlcfpmiNewInline);
			binary_io::write_uint32(stream, data.fcPlcflvcOld);
			binary_io::write_uint32(stream, data.lcbPlcflvcOld);
			binary_io::write_uint32(stream, data.fcPlcflvcOldInline);
			binary_io::write_uint32(stream, data.lcbPlcflvcOldInline);
			binary_io::write_uint32(stream, data.fcPlcflvcNew);
			binary_io::write_uint32(stream, data.lcbPlcflvcNew);
			binary_io::write_uint32(stream, data.fcPlcflvcNewInline);
			binary_io::write_uint32(stream, data.lcbPlcflvcNewInline);
			binary_io::write_uint32(stream, data.fcPgdMother);
			binary_io::write_uint32(stream, data.lcbPgdMother);
			binary_io::write_uint32(stream, data.fcBkdMother);
			binary_io::write_uint32(stream, data.lcbBkdMother);
			binary_io::write_uint32(stream, data.fcAfdMother);
			binary_io::write_uint32(stream, data.lcbAfdMother);
			binary_io::write_uint32(stream, data.fcPgdFtn);
			binary_io::write_uint32(stream, data.lcbPgdFtn);
			binary_io::write_uint32(stream, data.fcBkdFtn);
			binary_io::write_uint32(stream, data.lcbBkdFtn);
			binary_io::write_uint32(stream, data.fcAfdFtn);
			binary_io::write_uint32(stream, data.lcbAfdFtn);
			binary_io::write_uint32(stream, data.fcPgdEdn);
			binary_io::write_uint32(stream, data.lcbPgdEdn);
			binary_io::write_uint32(stream, data.fcBkdEdn);
			binary_io::write_uint32(stream, data.lcbBkdEdn);
			binary_io::write_uint32(stream, data.fcAfdEdn);
			binary_io::write_uint32(stream, data.lcbAfdEdn);
			binary_io::write_uint32(stream, data.fcAfd);
			binary_io::write_uint32(stream, data.lcbAfd);
		}
		if (data.nFib >= FibVersion::Fib2007)
		{
			//Read also the fibRgFcLcb2007
			binary_io::write_uint32(stream, data.fcPlcfmthd);
			binary_io::write_uint32(stream, data.lcbPlcfmthd);
			binary_io::write_uint32(stream, data.fcSttbfBkmkMoveFrom);
			binary_io::write_uint32(stream, data.lcbSttbfBkmkMoveFrom);
			binary_io::write_uint32(stream, data.fcPlcfBkfMoveFrom);
			binary_io::write_uint32(stream, data.lcbPlcfBkfMoveFrom);
			binary_io::write_uint32(stream, data.fcPlcfBklMoveFrom);
			binary_io::write_uint32(stream, data.lcbPlcfBklMoveFrom);
			binary_io::write_uint32(stream, data.fcSttbfBkmkMoveTo);
			binary_io::write_uint32(stream, data.lcbSttbfBkmkMoveTo);
			binary_io::write_uint32(stream, data.fcPlcfBkfMoveTo);
			binary_io::write_uint32(stream, data.lcbPlcfBkfMoveTo);
			binary_io::write_uint32(stream, data.fcPlcfBklMoveTo);
			binary_io::write_uint32(stream, data.lcbPlcfBklMoveTo);
			binary_io::write_uint32(stream, data.unknown21);
			binary_io::write_uint32(stream, data.unknown22);
			binary_io::write_uint32(stream, data.unknown23);
			binary_io::write_uint32(stream, data.unknown24);
			binary_io::write_uint32(stream, data.unknown25);
			binary_io::write_uint32(stream, data.unknown26);
			binary_io::write_uint32(stream, data.fcSttbfBkmkArto);
			binary_io::write_uint32(stream, data.lcbSttbfBkmkArto);
			binary_io::write_uint32(stream, data.fcPlcfBkfArto);
			binary_io::write_uint32(stream, data.lcbPlcfBkfArto);
			binary_io::write_uint32(stream, data.fcPlcfBklArto);
			binary_io::write_uint32(stream, data.lcbPlcfBklArto);
			binary_io::write_uint32(stream, data.fcArtoData);
			binary_io::write_uint32(stream, data.lcbArtoData);
			binary_io::write_uint32(stream, data.unknown27);
			binary_io::write_uint32(stream, data.unknown28);
			binary_io::write_uint32(stream, data.unknown29);
			binary_io::write_uint32(stream, data.unknown30);
			binary_io::write_uint32(stream, data.unknown31);
			binary_io::write_uint32(stream, data.unknown32);
			binary_io::write_uint32(stream, data.fcOssTheme);
			binary_io::write_uint32(stream, data.lcbOssTheme);
			binary_io::write_uint32(stream, data.fcColorSchemeMapping);
			binary_io::write_uint32(stream, data.lcbColorSchemeMapping);
		}

		binary_io::write_uint16(stream, data.cswNew);

		if (data.cswNew != 0)
		{
			//Read the FibRgCswNew
			binary_io::write_uint16(stream, data.nFibNew);
			binary_io::write_uint16(stream, data.cQuickSavesNew);
		}
		return stream;
	}
}
}