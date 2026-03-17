unit UMain;

interface

uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls, ExtCtrls, ComCtrls, Menus, IniFiles, Math,
  DXClass, DXDraws, DIB, ExtDlgs, IdBaseComponent, IdComponent, IdUDPBase,
  IdUDPClient, ShellAPI, jpeg;

const
  InstNum            = '8665';
  InstRevLevel       = '-';
  lblAddressWWW      = 'www.simtekinc.com';
  lblTestSoftwareCap = ': 10-' + InstNum;
  lblTestSoftwareRev = ':   ' + InstRevLevel;
  lblWARNINGCaption  = 'CAUTION FOR USE IN' +#$0D#$0A+ 'FLIGHT SIMULATOR ONLY';
  InstSec            = '10' + InstNum;
  icTXQue            = 512;
  icRXQue            = 1024;
  GENSec             = 'general';
  InstSecK01         = 'edtIPAddress';
  InstSecK02         = 'edtClientPort';
  InstSecK03         = 'cpdCOMBAUDRATE';
  GenSecK01          = 'TxWinLimit';
  GenSecK02          = 'RxWinLimit';
  GenSecK03          = 'cpdDATABITS';
  GenSecK04          = 'cpdPARITY';
  GenSecK05          = 'cpdSTOPBITS';
//  GenSecK06        = 'cpdHwFlow';
//  GenSecK07        = 'cpdSwFlow';
//  GenSecK08        = 'cpdEnableDTROnOpen';
//  GenSecK09        = 'cpdCheckLineStatus';
//  GenSecK10        = 'cpdInBufSize';
//  GenSecK11        = 'cpdOutBufSize';
// Host Command Setn to the Instrument
  TSCapHeader        = ' Transmit speed = ';

  C0Command          = 'Reset';
  C0Request          = $F0; // Reset Request
  C0Requestlength    = 2;
  C0CapHeader        = C0Command + ' Transmit speed = ';
  C0Response         = C0Request;
  C0Responselength   = 0;

//  C1Command          = 'Options';
//  C1Request          = $91; // Options Change Request
//  C1Requestlength    = 3;
//  C1CapHeader        = C1Command + ' Transmit speed = ';
//  C1Response         = C1Request;
//  C1Responselength   = 0;

  C2Command          = 'Firmware';
  C2Request          = $FE; // Firmware information
  C2Requestlength    = 2;
  C2CapHeader        = C2Command + ' Transmit speed = ';
  C2DeviceValDefault = ': 7?-???? Rev ?';
  C2Response         = C2Request;
  C2Responselength   = 14;

  C3Command          = 'Displays';
  C3Request          = $F5; // Display Request
  C3Requestlength    = 20;
  C3CapHeader        = C3Command + ' Transmit speed = ';
  C3Response         = C3Request;
  C3Responselength   = 0;

  C4Command          = 'Indicator';
  C4Request          = $F3; // Indicator/Annunciator Change Request
  C4Requestlength    = 3;
  C4CapHeader        = C4Command + ' Transmit speed = ';
  C4Response         = C4Request;
  C4Responselength   = 0;

  C5Command          = 'Dimming';
  C5Request          = $F4; // Dimming Change Request
  C5Requestlength    = 3;
  C5CapHeader        = C5Command + ' Transmit speed = ';
  C5Response         = C5Request;
  C5Responselength   = 0;

  C6Command          = 'Status';
  C6Request          = $F1; // Input Status Request
  C6Requestlength    = 6;
  C6CapHeader        = C6Command + ' Transmit speed = ';
  C6Response         = C6Request;
  C6Responselength   = 6;

  ScrollCapHeader    = 'Scroll Rate Value : ';
//  C7Command          = '';
//  C7Request          = $FE; // spare Change Request
//  C7Requestlength    = 0;
//  C7CapHeader        = C7Command + ' Transmit speed = ';
//  C7Response         = C7Request;
//  C7Responselength   = 0;

//  C8Command          = '';
//  C8Request          = $FF; // spare Change Request
//  C8Requestlength    = 0;
//  C8CapHeader        = C8Command + ' Transmit speed = ';
//  C8Response         = C8Request;
//  C8Responselength   = 0;

  // Descriptive labels to use with routines
  lblDimming1Cap     = 'Panel Dimming Level = ';
  lblDimming1Hint    = 'Shows Panel Brightness Value';
  lblDimming2Cap     = 'Display Dimming Level = ';
  lblDimming2Hint    = 'Shows Display Brightness Value';

  clActive           = clBlue;                                                  //Sets active color
  clInactive         = clBlack;                                                 //Sets inactive color
  lblBxbxValDefault  = ': unknown';

type
 Tx = record
   s    : string;                               //
   ai   : byte;                                 // average update rate index
   ar   : array[0..255] of double;              // average update rate
   us   : double;                               // update speed
   ui   : byte;                                 // update index
   uc   : integer;                              // update rate count
end;

type
  pbyte = ^byte;


type
  TMainForm = class(TDXForm)
    DXTimer1: TDXTimer;
    MainMenu1: TMainMenu;
    StatusBar: TStatusBar;
    pnlStatusLights: TPanel;
    PopupMenu1: TPopupMenu;
    Cut1: TMenuItem;
    Copy1: TMenuItem;
    Paste1: TMenuItem;
    Delete1: TMenuItem;
    Selectall1: TMenuItem;
    pmClearAll: TMenuItem;
    Print1: TMenuItem;
    IdUDPClient1: TIdUDPClient;
    mTXLight: TShape;
    lblTrxDataLight: TLabel;
    memoTx: TRichEdit;
    lblTransmitted: TLabel;
    tbUpdateRate: TTrackBar;
    lblTransmitRate: TLabel;
    cbxTRXWindowEnable: TCheckBox;
    mRXLight: TShape;
    lblRdxDataLight: TLabel;
    memoRx: TRichEdit;
    lblReceived: TLabel;
    lblRecievedTimeout: TLabel;
    tkbRecieveTimeOut: TTrackBar;
    cbxRDXWindowEnable: TCheckBox;
    mm01: TMenuItem;
    mm01s01: TMenuItem;
    mm02: TMenuItem;
    mm02s01: TMenuItem;
    mm02s02: TMenuItem;
    mm02s03: TMenuItem;
    mm03: TMenuItem;
    mm03s01: TMenuItem;
    mm03s02: TMenuItem;
    mm03s03: TMenuItem;
    mm03s04: TMenuItem;
    mm03s05: TMenuItem;
    mm03s05s02: TMenuItem;
    mm03s05s01: TMenuItem;
    mm03s05s03: TMenuItem;
    mm03s05s04: TMenuItem;
    mm03s05s05: TMenuItem;
    mm03s05s06: TMenuItem;
    mm04: TMenuItem;
    mm05: TMenuItem;
    mm06: TMenuItem;
    mm07: TMenuItem;
    mm08: TMenuItem;
    N1: TMenuItem;
    N2: TMenuItem;
    N3: TMenuItem;
    N4: TMenuItem;
    N5: TMenuItem;
    N6: TMenuItem;

    PageControl1: TPageControl;
    lblDimming1: TLabel;
    tkbDimming1: TTrackBar;
    cbxDimming1: TCheckBox;
    tkbDimming2: TTrackBar;
    lblDimming2: TLabel;
    cbxDimming2: TCheckBox;
    cbxResponse: TCheckBox;
    lblResponseRequestsTitle1: TLabel;
    lblResponseRequestsTitle2: TLabel;

    lblC1TransmitRate: TLabel;
    tkbC1UpdateRate: TTrackBar;
    lblC2TransmitRate: TLabel;
    tkbC2UpdateRate: TTrackBar;
    lblC3TransmitRate: TLabel;
    tkbC3UpdateRate: TTrackBar;
    lblC4TransmitRate: TLabel;
    tkbC4UpdateRate: TTrackBar;
    tkbC5UpdateRate: TTrackBar;
    lblC5TransmitRate: TLabel;
    tkbC6UpdateRate: TTrackBar;
    lblC6TransmitRate: TLabel;

    cbxGraphicsEnable: TCheckBox;
    ScrollBar2: TScrollBar;
    ScrollBar3: TScrollBar;
    lblRequestsSent: TLabel;
    lblResponseRecieved: TLabel;

    mm09: TMenuItem;
    mm10: TMenuItem;
    mm11: TMenuItem;
    mm12: TMenuItem;
    mm13: TMenuItem;
    mm14: TMenuItem;
    TabSheet1: TTabSheet;
    TabSheet4: TTabSheet;

    lblScrollRate: TLabel;
    lblTestSoftwarePNValue: TLabel;
    lblIPAddress: TLabel;
    lblReceivePort: TLabel;
    lblTestSoftwarePNTile: TLabel;
    lblTestSoftwareRevValue: TLabel;
    lblTestSoftwareRevTile: TLabel;
    imgSimtekLogo: TImage;
    edtIPAddress: TEdit;
    edtClientPort: TEdit;
    pnlDividerBar03: TPanel;
    pnlDividerBar01: TPanel;
    pnlDividerBar02: TPanel;
    pnlDividerBar04: TPanel;
    tkbScrollRate: TTrackBar;
    memoRevision: TMemo;
    cabinalt: TPanel;
    swSW01: TShape;
    swSW01Bat: TShape;
    Label16: TLabel;
    pnlDisplayRate: TPanel;
    edtRateX10000: TEdit;
    edtRateX01000: TEdit;
    edtRateX00100: TEdit;
    edtRateX00010: TEdit;
    edtRateX00001: TEdit;
    pnlDisplayCabinAlt: TPanel;
    edtCabinAltX10000: TEdit;
    edtCabinAltX01000: TEdit;
    edtCabinAltX00100: TEdit;
    edtCabinAltX00010: TEdit;
    edtCabinAltX00001: TEdit;
    Rate: TLabel;
    Label1: TLabel;
    pnlDisplayDifPress: TPanel;
    edtDifPressX100: TEdit;
    edtDifPressX010: TEdit;
    edtDifPressX001: TEdit;
    Label2: TLabel;
    pnlDisplayLDEConstAlt: TPanel;
    edtLDEConstAltX10000: TEdit;
    edtLDEConstAltX01000: TEdit;
    edtLDEConstAltX00100: TEdit;
    edtLDEConstAltX00010: TEdit;
    edtLDEConstAltX00001: TEdit;
    Label3: TLabel;
    Shape1: TShape;
    swValveControl: TShape;
    Shape3: TShape;
    Label4: TLabel;
    Shape4: TShape;
    Shape5: TShape;
    Shape6: TShape;
    Shape7: TShape;
    Shape8: TShape;
    Shape9: TShape;
    Shape10: TShape;
    Shape11: TShape;
    Shape12: TShape;
    Shape13: TShape;
    Shape14: TShape;
    Shape15: TShape;
    Shape16: TShape;
    labelOpen: TLabel;
    labelClose: TLabel;
    Label7: TLabel;
    Label8: TLabel;
    lblsw01t: TLabel;
    lblsw01b: TLabel;
    knobValveControl: TShape;
    knobAuto: TLabel;
    knobAuxVent: TLabel;
    knobConstAlt: TLabel;
    knobMan: TLabel;
    knobNoPress: TLabel;
    knobValveControlIndex: TShape;
    encoderLDEConstAlt: TShape;
    potAutoRate: TShape;
    Label11: TLabel;
    Shape18: TShape;
    Shape19: TShape;
    Label12: TLabel;
    Shape20: TShape;
    Shape21: TShape;
    Shape22: TShape;
    Shape23: TShape;
    Shape24: TShape;
    Shape25: TShape;
    Label13: TLabel;
    Label14: TLabel;
    Label15: TLabel;
    Label17: TLabel;
    Label18: TLabel;
    shapeClose: TShape;
    shapeOpen: TShape;
    btnLeftLDEConstAlt: TButton;
    btnRightLDEConstAlt: TButton;
    encoderLDEConstAltIndex: TShape;
    potAutoRateIndex: TShape;
    btnLeftAutoRate: TButton;
    btnRightAutoRate: TButton;
    decimalPoint: TShape;
    lblFirmwareTitle1: TLabel;
    lblFirmwareValue1: TLabel;
    lblFirmwareValue2: TLabel;
    lblFirmwareTitle2: TLabel;
    lblFirmwareTitle3: TLabel;
    lblFirmwareValue3: TLabel;
    pnlDividerBar05: TPanel;
    lblWARNING: TLabel;

    procedure FormCreate(Sender: TObject);
    procedure FormClose(Sender: TObject; var Action: TCloseAction);

    procedure PopupMenu1Popup(Sender: TObject);
    procedure Copy1Click(Sender: TObject);
    procedure Cut1Click(Sender: TObject);
    procedure Delete1Click(Sender: TObject);
    procedure Paste1Click(Sender: TObject);
    procedure Selectall1Click(Sender: TObject);
    procedure pmClearAllClick(Sender: TObject);
    procedure Print1Click(Sender: TObject);

    procedure DXTimer1Timer(Sender: TObject; LagCount: Integer);

    procedure memoTxChange(Sender: TObject);
    procedure memoRxChange(Sender: TObject);
    procedure memoTxKeyPress(Sender: TObject; var Key: char);
    procedure memoRxKeyPress(Sender: TObject; var Key: char);

    function ASCII27sEG(num: byte): byte;

    procedure tbUpdateRateChange(Sender: TObject);
    procedure lblFirmwareTitle1Click(Sender: TObject);
    procedure edtClientPortKeyPress(Sender: TObject; var Key: char);
    procedure edtIPAddressKeyPress(Sender: TObject; var Key: char);
    procedure tkbScrollRateChange(Sender: TObject);
    procedure cbxResponseClick(Sender: TObject);

    procedure tkbDimming1Change(Sender: TObject);
    procedure tkbDimming2Change(Sender: TObject);
    procedure tkbRecieveTimeOutChange(Sender: TObject);

    procedure mm01s01Click(Sender: TObject);
    procedure mm02s01Click(Sender: TObject);
    procedure mm03s01Click(Sender: TObject);
    procedure mm02s03Click(Sender: TObject);
    procedure mm03s03Click(Sender: TObject);
    procedure mm03s05s00Click(Sender: TObject);
    procedure mm04Click(Sender: TObject);
    procedure mm05Click(Sender: TObject);
    procedure mm06Click(Sender: TObject);
    procedure mm07Click(Sender: TObject);
    procedure mm08Click(Sender: TObject);
    procedure memoRevisionEnter(Sender: TObject);
    procedure memoRevisionMouseLeave(Sender: TObject);
    procedure ScrollBar2Change(Sender: TObject);
    procedure ScrollBar3Change(Sender: TObject);
    procedure imgSimtekLogoClick(Sender: TObject);
    procedure shpIndicatorD0MouseUp(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure edtRateX10000Change(Sender: TObject);
    procedure edtRateX10000Click(Sender: TObject);
    procedure edtCabinAltX10000Change(Sender: TObject);
    procedure edtCabinAltX10000Click(Sender: TObject);
    procedure edtLDEConstAltX10000Change(Sender: TObject);
    procedure edtLDEConstAltX10000Click(Sender: TObject);
    procedure edtDifPressX100Change(Sender: TObject);
    procedure edtDifPressX001Change(Sender: TObject);
    procedure edtDifPressX100Click(Sender: TObject);
    procedure memoRevisionExit(Sender: TObject);
    procedure mm09Click(Sender: TObject);
    procedure mm10Click(Sender: TObject);
    procedure mm11Click(Sender: TObject);
    procedure mm12Click(Sender: TObject);
    procedure swSW01BatMouseUp(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure swValveControlMouseUp(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure knobValveControlMouseUp(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure btnLeftLDEConstAltMouseUp(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure btnRightLDEConstAltMouseUp(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure btnLeftAutoRateMouseUp(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure btnRightAutoRateMouseUp(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure encoderLDEConstAltMouseUp(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure decimalPointMouseUp(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure encoderLDEConstAltMouseDown(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure TabSheet1Enter(Sender: TObject);
  private

    procedure setToggleSwitch(pos, chg : byte; bat, lblT, lblM, lblB: TObject);
    procedure setRotarySwitch(bd,ix,lbl0,lbl1,lbl2,lbl3,lbl4: TObject;stp: byte);
    procedure setEncoder(bd, ix: TObject;stp: byte);
    procedure setEncoderPress(button : TObject);
    procedure setPot(bd, ix: TObject;stp: integer);
    procedure setRockerSwitch(pos : byte; button, indexTop, indexBottom, lblTop, lblBottom : TObject);
    procedure paintGUI;
    procedure decodeRecievedData(s : string);
    procedure calculateRotaryIndexPos(rot, indx: TObject;stops: integer);
    procedure calculateEncoderIndexPos(rot, indx: TObject;stops: integer);
    procedure calculatePotIndexPos(rot, indx: TObject;offset, stops: integer);
    function  encodeCommandData(CommandByte : byte) : string;
    function  buildMainCaption : String;
    function  calculateRate(ct : double;g : Tx;rc : boolean;l : TObject;t : TObject;s : string):Tx;
    procedure cpOutputData(s : string; rc : boolean);

    procedure responseReset;
    procedure responseStatus(ptrCK: pbyte);
    procedure responseFirmware(s : string);

  public
//    procedure DisplayHint1(Sender: TObject);
    procedure setColorCOMLightTX(SetColor : TColor;i : integer);
    procedure setColorCOMLightRX(SetColor : TColor;i : integer);
    procedure TrafficLightsOff;
  end;

var
  MainForm                 : TMainForm;
  hWind                    : THandle;
// Variables to control Software functions
  dScrollValue             : byte    = 1;
  ScrollCount              : integer = 0;
  TXCount                  : byte    = 1; {define and initilize Counter}
  RXCount                  : byte    = 1; {define and initilize Counter}
  sWidth                   : integer = 1024;
// Software control variables
  RxWinLimit               : integer = 0;
  RxPlace                  : integer = 0;
  TxWinLimit               : integer = 0;
  TxPlace                  : integer = 0;
// Device Output control variables
  fImageRedraw             : boolean;

  ActiveDigit              : cardinal = 0;

  C0RequestData            : array[0..C0Requestlength] of byte;                 // reset
//  C1RequestData            : array[0..C1Requestlength] of byte;               // options
  C2RequestData            : array[0..C2Requestlength] of byte;                 // program revision
  C3RequestData            : array[0..C3Requestlength] of byte;                 // display
  C4RequestData            : array[0..C4Requestlength] of byte;                 // indicator
  C5RequestData            : array[0..C5Requestlength] of byte;                 // dimmimg
  C6RequestData            : array[0..C6Requestlength] of byte;                 // status
//  C7RequestData            : array[0..C7Requestlength] of byte;               //
//  C8RequestData            : array[0..C8Requestlength] of byte;

//  C0ResponseData           : array[0..C0Responselength] of byte;              // reset
//  C1ResponseData           : array[0..C1Responselength] of byte;              // options
  C2ResponseData           : array[0..C2Responselength] of byte;                // program revision
//  C3ResponseData           : array[0..C3Responselength] of byte;              // display
//  C4ResponseData           : array[0..C4Responselength] of byte;              // indicator
//  C5ResponseData           : array[0..C5Responselength] of byte;              // dimming
  C6ResponseData           : array[0..C6Responselength] of byte;                // status
//  C7ResponseData           : array[0..C7Responselength] of byte;              //
//  C8ResponseData           : array[0..C8Responselength] of byte;
// Host to Instrument request Action Flags
  fC0Request               : boolean  = False;                                  // reset
  fC1Request               : boolean  = False;                                  // options
  fC2Request               : boolean  = False;                                  // program revision
  fC3Request               : boolean  = False;                                  // display
  fC4Request               : boolean  = false;                                  // indicator
  fC5Request               : boolean  = false;                                  // dimming
  fC6Request               : boolean  = false;                                  // status
//  fC7Request               : boolean  = false;                                //
//  fC8Request               : boolean  = false;                                //
  fScrollReq               : boolean  = False;                                  // scroll change flag
  fgeneric                 : boolean  = False;

// Device Input control variables
  swMode                   : byte     = 1;
  swPreset                 : byte     = 1;

// Device BIT control variables
  fPTInputStatus           : boolean  = False;
  fCTInputStatus           : boolean  = False;
  fRKInputStatus           : boolean  = False;
  fOFLInputStatus          : boolean  = False;
  fEBInputStaus            : boolean  = False;
  fZALLInputStatus         : boolean  = False;
  fPresetPWROFFInputStatus : boolean  = False;
  fPresetMANInputStatus    : boolean  = False;
  fPreset1InputStatus      : boolean  = False;
  fPreset2InputStatus      : boolean  = False;
  fPreset3InputStatus      : boolean  = False;
  fPreset4InputStatus      : boolean  = False;
  fPreset5InputStatus      : boolean  = False;
  fPreset6InputStatus      : boolean  = False;
  fPresetREMInputStatus    : boolean  = False;
  fINITInputStatus         : boolean  = False;
  fRightArrowInputStatus   : boolean  = False;
  fUpArrowInputStatus      : boolean  = False;
  fBRTDSPLA2DInputStatus   : boolean  = False;
  fBRTPNLA2DInputStatus    : boolean  = False;
  fMemoryStatus            : boolean  = False;
// Display Dimming Variables
  tkbDimming1Old           : integer;
  tkbDimming2Old           : integer;
// Debug Variables
  NumberOfRequests         : cardinal = 0;
  UserImageSelected        : boolean  = True;
  NumberOfResponse         : cardinal = 0;
  gRate                    : Tx;
  gRateC1                  : Tx; // StatusRequest      = $F1;
  gRateC2                  : Tx; // DisplayRequest     = $F2;
  gRateC3                  : Tx; // DimmingRequest     = $F3;
  gRateC4                  : Tx; // FirmwareRequest    = $F4;
  gRateC5                  : Tx; // FirmwareRequest    = $F4;
  gRateC6                  : Tx; // FirmwareRequest    = $F4;
  gRateScroll              : Tx;
  BurstFileName            : string = 'BurstFile.txt';
  BurstFileContents        : TStringlist = nil;
  //bit to reset procedures
  resetbit                  : boolean;               //


////////////////////////////////////////////////////////////////////////////////
///  Very bad practice, but this is a global variable for "Encoder Pressed"  ///
////////////////////////////////////////////////////////////////////////////////

encoderPressed             : byte = $0;


implementation

{$R *.DFM}

procedure TMainForm.FormCreate(Sender: TObject);
var SimtekIni : TIniFile;
    tmp : integer;
begin
  SimtekIni := TIniFile.Create('C:\Simtek.Ini');
  With SimtekIni do
    begin
      // load the limit for the transmit and recieve windows
    TxWinLimit         := ReadInteger(GENSec,  GenSecK01,  250);      //
    RxWinLimit         := ReadInteger(GENSec,  GenSecK02,  250);      //
    edtIPAddress.Text  := ReadString(InstSec,  InstSecK01, '192.168.136.90');   // {Set the default flags for startup          }
    edtClientPort.Text := ReadString(InstSec,  InstSecK02, '51020');            //{Set the default flags for startup          }
    IdUDPClient1.Port  := StrToInt(edtClientPort.Text);   
    end;
  SimtekIni.Free;

  ////////////////////check for burstfile.txt for menu//////////////////////////
  if fileexists('burstfile.txt') then
  mm05.visible := True
  else mm05.visible := False;

  TxPlace   := 1;
  tmp       := TxWinLimit;
  while tmp > 10 do
    begin
    tmp     := tmp div 10;
    TxPlace := TxPlace + 1;
    end;
  tmp       := RxWinLimit;     
  while tmp > 10 do
    begin
    tmp     := tmp div 10;
    RxPlace := RxPlace + 1;
    end;

  {---------------- ATTENTION ----------------------------------------------}
  { initialize your variables here. DO NOT place them after the Port1click  }
  { Routine as the communications events will be active and you may get an  }
  { invalid value if your variables are not initialized prior to starting   }
  { the communications events                                               }
  {--------------- ATTENTION ^ READ IT -------------------------------------}
  responseReset;
  fImageRedraw                    := True;
  DXTimer1.Enabled                := True;
end;

procedure TMainForm.imgSimtekLogoClick(Sender: TObject);                        ////////////////////////////////////////////////
var url : string;                                                               // Clicking on the Simtek Logo will launch the//
begin                                                                           // Simtek Website                             //
  url := lblAddressWWW;                                                         //                                            //
  ShellExecute(self.WindowHandle,'open',PChar(url),nil,nil, SW_SHOWNORMAL);     ////////////////////////////////////////////////
end;

procedure TMainForm.knobValveControlMouseUp(Sender: TObject;
  Button: TMouseButton; Shift: TShiftState; X, Y: Integer);                     // Mouseup on round shape creating rotation
  var pos  : integer;                                                           // of a "knob"
begin                                                                           //
  pos := knobValveControl.Tag shl 1;                                            // uses tag to shift position
  if(pos > $10)then pos := 1;                                                   // $10 is the last position on the rotary switch
  knobValveControl.Tag := pos;                                                  // (this hex value will change per rotary switch)
  if(pos < $10)then knobValveControl.Tag := pos;                                // shifts bit left to increase position of "Index"
  knobValveControlIndex.Visible := True;
  fImageRedraw := True;
end;

procedure TMainForm.FormClose(Sender: TObject; var Action: TCloseAction);
begin
  Action := caFree;                                     //
end;

function TMainForm.buildMainCaption: string;
var s  : string;
begin
  s := 'Test Software ' + lblTestSoftwareCap + '   IP = ' + edtIPAddress.Text + '   Device Port = ' + edtClientPort.Text;
  Result := s;
end;

function TMainForm.encodeCommandData(CommandByte: byte): string;
var
   s       : string;
   tmp     : integer;
   i       : byte;
begin
  s := '';
  case CommandByte of
    C0Request  : begin               // reset
      C0RequestData[1] := CommandByte;
      s        := chr(C0RequestData[1]);
      end;
    C2Request  : begin               // firmware
      C2RequestData[1] := CommandByte;
      s        := chr(C2RequestData[1]);  // 01
      inc(NumberOfRequests);
    end;
    C3Request  : begin                // display
      C3RequestData[1]    := CommandByte;
      for i := 1 to (C3Requestlength - 1) do
        s   := s + Chr(C3RequestData[i]);         // xx
      end;
    C5Request  : begin               // dimming
      C5RequestData[1] := CommandByte;
      s        := chr(C5RequestData[1]);   // 01
      tmp      := tkbDimming1.Position;
      s        := s + chr(tmp);
      tmp      := tkbDimming2.Position;
      s        := s + chr(tmp);
      end;
    C6Request : begin               // status
      s        := chr(CommandByte);   // 01
      inc(NumberOfRequests);
      end;
    else begin
      s   := chr(CommandByte);
    end;
  end;
  Result  := s;
end;

procedure TMainForm.encoderLDEConstAltMouseDown(Sender: TObject;
  Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
var status : byte;
begin
  if encoderPressed = $0 then
    begin
      Tshape(Sender).Brush.Color := clGray;
      encoderPressed := $10;
    end
  else if encoderPressed = $10 then
    begin
      Tshape(Sender).Brush.Color := clBlack;
      encoderPressed := $0;
    end

end;

procedure TMainForm.encoderLDEConstAltMouseUp(Sender: TObject;                  // this mouseup needs to send a signal
  Button: TMouseButton; Shift: TShiftState; X, Y: Integer);                     //
begin                                                                           // Isn't being used right now
  TShape(encoderLDEConstAlt).Tag := $10;                                        //
end;                                                                            //

procedure TMainForm.setColorCOMLightRX(SetColor: TColor;i : integer);
begin
  if i > 1 then Dec(i);
  if i = 1 then
    begin
    mRXLight.Brush.Color := SetColor;                                           // change the comm transmit light to active
    end;
end;

procedure TMainForm.setColorCOMLightTX(SetColor: TColor;i : integer);
begin
  if i > 1 then Dec(i);
  if i = 1 then
    begin
    TXCount := 0;
    mTXLight.Brush.Color := SetColor;                                           // change the comm transmit light to active
    end;
end;

procedure TMainForm.setToggleSwitch(pos, chg : byte; bat, lblT, lblM, lblB: TObject);
begin
  TLabel(lblT).Font.Color := clInactive;
  if lblM <> nil then TLabel(lblM).Font.Color := clInactive;
  TLabel(lblB).Font.Color := clInactive;
  with TShape(bat) do
    begin
    if resetbit then pos := 0;
    case pos of
      $01 : begin TLabel(lblB).Font.Color := clActive;   TShape(bat).Top := TShape(bat).Tag + chg;         end;
      $02 : begin TLabel(lblT).Font.Color := clActive;   TShape(bat).Top := TShape(bat).Tag - chg;         end;
      $04 : begin TLabel(lblB).Font.Color := clActive;   TShape(bat).Top := TShape(bat).Tag + chg;         end;
      $08 : begin TLabel(lblT).Font.Color := clActive;   TShape(bat).Top := TShape(bat).Tag - chg;         end;
      $10 : begin TLabel(lblB).Font.Color := clActive;   TShape(bat).Top := TShape(bat).Tag + chg;         end;
      $20 : begin TLabel(lblT).Font.Color := clActive;   TShape(bat).Top := TShape(bat).Tag - chg;         end;
      $40 : begin TLabel(lblB).Font.Color := clActive;   TShape(bat).Top := TShape(bat).Tag + chg;         end;
      $80 : begin TLabel(lblT).Font.Color := clActive;   TShape(bat).Top := TShape(bat).Tag - chg;         end;
      else  begin TLabel(lblB).Font.Color := clInactive; TShape(bat).Top := TShape(bat).Tag;               end;
      end;//case
    end;//with
end;

procedure TMainForm.setRotarySwitch(bd,ix,lbl0,lbl1,lbl2,lbl3,lbl4: TObject;stp: byte);

begin
  calculateRotaryIndexPos(bd,ix,stp);                                           // sw body shape, sw index shape, num of sw positions
  if lbl0 <> nil then TLabel(lbl0).Font.Color := clInactive;
  if lbl1 <> nil then TLabel(lbl1).Font.Color := clInactive;
  if lbl2 <> nil then TLabel(lbl2).Font.Color := clInactive;
  if lbl3 <> nil then TLabel(lbl3).Font.Color := clInactive;
  if lbl4 <> nil then TLabel(lbl4).Font.Color := clInactive;
  case TShape(bd).Tag of
    $00 :   if lbl0 <> nil then TLabel(lbl0).Font.Color := clActive;
    $01 :   if lbl0 <> nil then TLabel(lbl0).Font.Color := clActive;
    $02 :   if lbl1 <> nil then TLabel(lbl1).Font.Color := clActive;
    $04 :   if lbl2 <> nil then TLabel(lbl2).Font.Color := clActive;
    $08 :   if lbl3 <> nil then TLabel(lbl3).Font.Color := clActive;
    $10 :   if lbl4 <> nil then TLabel(lbl4).Font.Color := clActive;
  end;
  if resetbit then Tag := 0;
end;

procedure TMainForm.setEncoder(bd, ix: TObject;stp: byte);

begin
  calculateEncoderIndexPos(bd,ix,stp);
end;

procedure TMainForm.setEncoderPress(button : TObject);

begin
  TShape(button).Brush.Color := clBlack;
  with TShape(button) do
    begin
      if resetbit then encoderPressed := $0;
        case encoderPressed of
          $0  : begin TShape(button).Brush.Color := clBlack; end;
          $10 : begin TShape(button).Brush.Color := clGray; end;
        end// case
    end// with
end;

procedure TMainForm.setPot(bd, ix: TObject;stp: integer);

begin
  calculatePotIndexPos(bd, ix, 30, stp);                                        // 30 is put in manually as an offset
end;

////////////////////////////////////////////////////////////////////////////////
//  There is one shape for this rocker switch, tag and thereby pos is set     //
//  in the mouseup function for the rocker.                                   //
//  There are only 2 states and this uses an "Index" shape and highlighted    //
//  text to show which state of the switch is active.                         //
////////////////////////////////////////////////////////////////////////////////

procedure TMainForm.setRockerSwitch(pos :byte; button, indexTop, indexBottom, lblTop, lblBottom: TObject);
begin
  TLabel(lblTop).Font.Color := clInactive;
  TLabel(lblBottom).Font.Color := clInactive;
  
  with TShape(button) do
    begin
      if resetbit then pos := 0;
          case pos of
            $00: begin
                 TShape(indexTop).Visible := False;     Tshape(indexBottom).Visible := False;
                 //pos := pos + $01;
                 end;//end $00
            $01 : begin TLabel(lblTop).Font.Color := clActive;      TShape(indexTop).Visible := True;     Tshape(indexBottom).Visible := False;   end;
            $02 : begin TLabel(lblBottom).Font.Color := clActive;   TShape(indexBottom).Visible := True;  Tshape(indexTop).Visible := False;      end;
          end;//case
    end
  end;


procedure TMainForm.shpIndicatorD0MouseUp(Sender: TObject; Button: TMouseButton;
  Shift: TShiftState; X, Y: Integer);
begin
  if((C4RequestData[2] and TLabel(Sender).Tag) = TLabel(Sender).Tag)then
    C4RequestData[2] := C4RequestData[2] and (not TLabel(Sender).Tag)
  else
    C4RequestData[2] := C4RequestData[2] or TLabel(Sender).Tag;
  fC4Request := True;
  fImageRedraw := True;
end;


procedure TMainForm.swSW01BatMouseUp(Sender: TObject; Button: TMouseButton;     /////////////////////////////////////
  Shift: TShiftState; X, Y: Integer);                                           // Responsible for counting through//
var i : integer;                                                                // the 3 positions of the toggle   //
begin                                                                           // switch and re-drawing after     //
  i :=   TShape(Sender).Tag + 1;                                                // each click                      //
  if i = 3 then i := 0;                                                         /////////////////////////////////////
  TShape(Sender).Tag := i;
  fImageRedraw := True;
end;

procedure TMainForm.swValveControlMouseUp(Sender: TObject; Button: TMouseButton;// swValveControl is a toggle switch
  Shift: TShiftState; X, Y: Integer);                                           // this procedure gets out of the initial
var holder1 : TLabel;                                                           // blank state (both labels white) and
var holder2 : TShape;
var pos     : integer;                                                          // then alternates back and forth, simulating
begin                                                                           // the rocker going from one state to the next

  pos := swValveControl.Tag shl 1;
  if(pos > $02)then pos := 1;                                                   // only 2 states for a rocker switch
  swValveControl.Tag := pos;
  if(pos < $02)then swValveControl.Tag := pos;
    begin
      if pos = $0 then
        begin
          if ((labelClose.Font.Color = clBlue) and (shapeClose.Visible = True)) then
            begin
              labelClose.Font.Color := clBlue;
              shapeClose.Visible := True;
              labelOpen.Font.Color := clWhite;
              shapeOpen.Visible := False;
            end
          else if ((labelOpen.Font.Color = clBlue) and (shapeOpen.Visible = True)) then
            begin
              labelOpen.Font.Color := clBlue;
              shapeOpen.Visible := True;
              labelClose.Font.Color := clWhite;
              shapeClose.Visible := False;
            end;
        end
      else if pos = $01 then
        begin
          labelClose.Font.Color := clWhite;
          shapeClose.Visible := False;
          labelOpen.Font.Color := clBlue;
          shapeOpen.Visible := True;
        end
      else
        begin
          labelOpen.Font.Color := clWhite;
          shapeOpen.Visible := False;
          labelClose.Font.Color := clBlue;
          shapeClose.Visible := True;
        end
    end;
  fImageRedraw := True;
end;

procedure TMainForm.TabSheet1Enter(Sender: TObject);
begin
  ActiveControl := edtIPAddress;
end;

procedure TMainForm.TrafficLightsOff;
begin
  setColorCOMLightTX(clBtnFace,1);                                              // change the comm recieve light to inactive
  setColorCOMLightRX(clBtnFace,1);                                              // change the comm recieve light to inactive
end;

procedure TMainForm.memoRxChange(Sender: TObject);
begin
  lblReceived.Caption := 'Lines Received : ' + IntToStr(memoRx.lines.count);
end;

procedure TMainForm.memoRxKeyPress(Sender: TObject; var Key: char);
var UserrecieveString,CharacterCreationString,StringToRecieve : string;
    CharacterPointer,CharacterToRecieve,Stringlength,BadCharPos : byte;
begin
  case Key of
    '0'..'9':;
    'A'..'F':;
    'a'..'f':;
    #8 :;
    #13:begin
        UserrecieveString := memoRx.Lines.Strings[memoRx.Lines.Count-1];
        memoRx.Lines.Delete(memoRx.Lines.Count-1);
        BadCharPos := pos('>',UserrecieveString);
        if BadCharPos <> 0 then
          begin
          memoRx.Lines.Add(UserrecieveString);
          Delete(UserrecieveString,1,BadCharPos+1);
          end;
        While pos(' ',UserrecieveString) <> 0 do
          begin
          BadCharPos := pos(' ',UserrecieveString);
          Delete(UserrecieveString,BadCharPos,1);
          end;//while
        Stringlength := length(UserrecieveString);
        CharacterPointer := 1;
        StringToRecieve := '';
        While CharacterPointer <= Stringlength do
          begin
          CharacterCreationString := '$' + UserrecieveString[CharacterPointer] + UserrecieveString[CharacterPointer+1];
          CharacterToRecieve := StrToInt(CharacterCreationString);
          StringToRecieve := StringToRecieve + Chr(CharacterToRecieve);
          CharacterPointer := CharacterPointer + 2;
          end;
        if StringToRecieve <> '' then decodeRecievedData(StringToRecieve);
        Key := #00;
        end;
    else Key := #00;
  end;//case
end;

procedure TMainForm.memoTxChange(Sender: TObject);
begin
  lblTransmitted.Caption := 'Lines Transmitted : ' + IntToStr(memoTx.lines.count);
end;

procedure TMainForm.memoTxKeyPress(Sender: TObject; var Key: char);
var UserTransmitString,CharacterCreationString,StringToTransmit : string;
    CharacterPointer,CharacterToTransmit,Stringlength,BadCharPos : byte;
begin
 case Key of
  '0'..'9':;
  'A'..'F':;
  'a'..'f':;
  #8:;
  #13:begin
      UserTransmitString := memoTx.Lines.Strings[memoTx.Lines.Count-1];
      memoTx.Lines.Delete(memoTx.Lines.Count-1);
      BadCharPos := pos('>',UserTransmitString);
      if BadCharPos <> 0 then
       begin
       memoTx.Lines.Add(UserTransmitString);
       Delete(UserTransmitString,1,BadCharPos+1);
       end;
      While pos(' ',UserTransmitString) <> 0 do
       begin
       BadCharPos := pos(' ',UserTransmitString);
       Delete(UserTransmitString,BadCharPos,1);
       end;//while
      Stringlength := length(UserTransmitString);
      CharacterPointer := 1;
      StringToTransmit := '';
      While CharacterPointer <= Stringlength do
       begin
       CharacterCreationString := '$' + UserTransmitString[CharacterPointer] + UserTransmitString[CharacterPointer+1];
       CharacterToTransmit := StrToInt(CharacterCreationString);
       StringToTransmit := StringToTransmit + Chr(CharacterToTransmit);
       CharacterPointer := CharacterPointer + 2;
       end;
      if StringToTransmit <> '' then cpOutputData(StringToTransmit, False);
      if cbxResponse.Checked then
        begin
        inc(NumberOfRequests);
        end;
      Key := #00;
      end;
  else Key := #00;
  end;//case
end;

procedure TMainForm.cpOutputData(s: string; rc: boolean);
var
   i         : integer;
   count     : byte;
   SMsg      : string;
   lsLineNum : string;
begin
  if s <> '' then                                                               //
    begin
    IdUDPClient1.Host := edtIPAddress.Text;                                     //
    IdUDPClient1.Send(s);                                                       //
    TXCount := 30;                                                              //
    setColorCOMLightTX(clLime,1);                                               //
    end;
  if cbxTRXWindowEnable.Checked then                                            //
    begin
    count := length(s);                                                         //
    for i := 1 to count do                                                      // Loop through the receive buffer and
      begin
      SMsg := SMsg + IntToHex(ord(s[i]),2)+' ';                                 // Convert the bytes to a pascal string
      end;
    if memoTx.Lines.Count >= TxWinLimit then
      memoTx.Clear;
    lsLineNum := 'TX ' + IntToStr(memoTx.Lines.Count) + ' -> ';
    While length(lsLineNum) < (TxPlace + 7) do
      Insert('0',lsLineNum,4);
    memoTx.Lines.Add(lsLineNum + SMsg);           {then store them in the TX Window }
    lblTransmitted.Caption := 'Lines Transmitted : ' + IntToStr(memoTx.Lines.Count);
    end;
end;

procedure TMainForm.tkbDimming1Change(Sender: TObject);
begin
  if tkbDimming1.Position <> tkbDimming1Old then
    begin
    tkbDimming1Old      := tkbDimming1.Position;
    lblDimming1.Caption := lblDimming1Cap + IntToStr(tkbDimming1Old);
    if not cbxDimming1.Checked then fC5Request := True;
    end;
end;

procedure TMainForm.tkbDimming2Change(Sender: TObject);
begin
  if tkbDimming2.Position <> tkbDimming2Old then
    begin
    tkbDimming2Old      := tkbDimming2.Position;
    lblDimming2.Caption := lblDimming2Cap + IntToStr(tkbDimming2Old);
    if not cbxDimming2.Checked then fC5Request := True;
    end;
end;

procedure TMainForm.tkbRecieveTimeOutChange(Sender: TObject);
begin
  lblRecievedTimeout.Caption := 'Recieve Timeout = ' + IntToStr(tkbRecieveTimeOut.Position);
end;

procedure TMainForm.tkbScrollRateChange(Sender: TObject);
begin
 if tkbScrollRate.Position < tbUpdateRate.Position then
   tbUpdateRate.Position := tkbScrollRate.Position;
 lblScrollRate.caption   := 'Scroll Rate Value: ' + InttoStr(tkbScrollRate.Position);
end;

procedure TMainForm.Cut1Click(Sender: TObject);
begin
  with TRichEdit(Sender) do
   begin
    Try
      CutToClipBoard;
    Except
      Beep;
    End;
   end;
end;

procedure TMainForm.Copy1Click(Sender: TObject);
begin
  with TRichEdit(Sender) do
   begin
    Try
      CopyToClipboard;
    Except
      Beep;
    End;
   end;
end;

procedure TMainForm.Paste1Click(Sender: TObject);
begin
  with TRichEdit(Sender) do
   begin
    Try
      PasteFromClipboard;
    Except
      Beep;
    End;
   end;
end;

procedure TMainForm.Delete1Click(Sender: TObject);
begin
  with TRichEdit(Sender) do
   begin
    Try
      ClearSelection;
    Except
      Beep;
    End;
   end;
end;

procedure TMainForm.Selectall1Click(Sender: TObject);
begin
  with TRichEdit(Sender) do
   begin
    Try
      SelectAll;
    Except
      Beep;
    End;
   end;
end;

procedure TMainForm.pmClearAllClick(Sender: TObject);
begin
  with TRichEdit(Sender) do
   begin
    Try
      Clear;
    Except
      Beep;
    End;
   end;
end;

procedure TMainForm.Print1Click(Sender: TObject);
var s : string;
begin
  with TRichEdit(Sender) do
   begin
    Try
      s                           := Name;
      Print(s + ' Contents');
    Except
      on EInOutError do
        begin
        MessageDlg('Error Printing Text',mtError,[mbOk],0);
        Beep;
        end;
    End;
   end;
end;

procedure TMainForm.cbxResponseClick(Sender: TObject);
var ofset : integer;
begin
  lblResponseRequestsTitle1.Visible := cbxResponse.Checked;
  lblRequestsSent.Visible           := cbxResponse.Checked;
  lblResponseRequestsTitle2.Visible := cbxResponse.Checked;
  lblResponseRecieved.Visible       := cbxResponse.Checked;
  lblRecievedTimeout.Visible        := cbxResponse.Checked;
  tkbRecieveTimeOut.Visible         := cbxResponse.Checked;
  NumberOfRequests                  :=   0;
  NumberOfResponse                  :=   0;

  if cbxResponse.Checked then ofset :=  60
  else                        ofset :=   0;
end;

procedure TMainForm.ScrollBar2Change(Sender: TObject);
//r ofset : integer;
begin
    lblTransmitRate.Top           :=  10 - ScrollBar2.Position;
    tbUpdateRate.Top              :=  22 - ScrollBar2.Position;
    tkbC1UpdateRate.Top           :=  64 - ScrollBar2.Position;
    lblC1TransmitRate.Top         :=  52 - ScrollBar2.Position;
    lblC2TransmitRate.Top         :=  94 - ScrollBar2.Position;
    tkbC2UpdateRate.Top           := 106 - ScrollBar2.Position;
    lblC3TransmitRate.Top         := 136 - ScrollBar2.Position;
    tkbC3UpdateRate.Top           := 148 - ScrollBar2.Position;
    lblC4TransmitRate.Top         := 178 - ScrollBar2.Position;
    tkbC4UpdateRate.Top           := 190 - ScrollBar2.Position;
    lblC5TransmitRate.Top         := 220 - ScrollBar2.Position;
    tkbC5UpdateRate.Top           := 232 - ScrollBar2.Position;
    lblC6TransmitRate.Top         := 262 - ScrollBar2.Position;
    tkbC6UpdateRate.Top           := 270 - ScrollBar2.Position;
    cbxTRXWindowEnable.Top        := 310 - ScrollBar2.Position;
    cbxRDXWindowEnable.Top        := 330 - ScrollBar2.Position;
    cbxGraphicsEnable.Top         := 350 - ScrollBar2.Position;
    cbxResponse.Top               := 370 - ScrollBar2.Position;
    lblResponseRequestsTitle1.Top := 390 - ScrollBar2.Position;
    lblRequestsSent.Top           := 390 - ScrollBar2.Position;
    lblResponseRequestsTitle2.Top := 410 - ScrollBar2.Position;
    lblResponseRecieved.Top       := 410 - ScrollBar2.Position;
    lblRecievedTimeout.Top        := 430 - ScrollBar2.Position;
    tkbRecieveTimeOut.Top         := 430 - ScrollBar2.Position;
//  if cbxResponse.Checked then   ofset :=  60
//  else                          ofset :=   0;
//    lblPot2TrapZero.caption       := IntToStr(ScrollBar2.Position);
end;

procedure TMainForm.ScrollBar3Change(Sender: TObject);
begin
  lblDimming1.Top                 :=  10 - ScrollBar3.Position;
  cbxDimming1.Top                 :=   8 - ScrollBar3.Position;
  tkbDimming1.Top                 :=  24 - ScrollBar3.Position;
  lblDimming2.Top                 :=  52 - ScrollBar3.Position;
  cbxDimming2.Top                 :=  50 - ScrollBar3.Position;
  tkbDimming2.Top                 :=  66 - ScrollBar3.Position;
//  lblDimming3.Top                 :=  94 - ScrollBar3.Position;
//  cbxDimming3.Top                 :=  92 - ScrollBar3.Position;
//  tkbDimming3.Top                 := 108 - ScrollBar3.Position;
end;

procedure TMainForm.edtClientPortKeyPress(Sender: TObject; var Key: char);
var port      : integer;
    SimtekIni : TIniFile;
begin
 case Key of
  '0'..'9':;
  #8:;
  #13:begin
      port                        := StrToInt(edtClientPort.Text);
      IdUDPClient1.Port           := port;
      with TEdit(Sender) do
        begin
        SimtekIni                 := TIniFile.Create('C:\Simtek.Ini');
        with SimtekIni do begin   WriteString(InstSec, InstSecK02, edtClientPort.Text); end; {Set the default flags for startup          }
        SimtekIni.Free;
        end;//with TEdit
      MainForm.Caption            := buildMainCaption;       //
      end//case #13
  else Key := #00;
  end;//case
end;

function TMainForm.ASCII27sEG(num: byte): byte;
begin
    case num of
      $30 : result := $3f;                            //0
      $31 : result := $06;                            //1
      $32 : result := $5B;                            //2
      $33 : result := $4F;                            //3
      $34 : result := $66;                            //4
      $35 : result := $6D;                            //5
      $36 : result := $7D;                            //6
      $37 : result := $07;                            //7
      $38 : result := $7F;                            //8
      $39 : result := $6F;                            //9
      $61 : result := $01;                            //a seg
      $62 : result := $02;                            //b seg
      $63 : result := $04;                            //c seg
      $64 : result := $08;                            //d seg
      $65 : result := $10;                            //e seg
      $66 : result := $20;                            //f seg
      $67 : result := $40;                            //g seg

      $B0 : result := $BF;                            //.0
      $B1 : result := $46;                            //.1
      $B2 : result := $DB;                            //.2
      $B3 : result := $CF;                            //.3
      $B4 : result := $E6;                            //.4
      $B5 : result := $ED;                            //.5
      $B6 : result := $FD;                            //.6
      $B7 : result := $87;                            //.7
      $B8 : result := $FF;                            //.8
      $B9 : result := $EF;                            //.9
      else  result := $00;
    end;
end;


procedure TMainForm.edtRateX10000Change(Sender: TObject);
var s : string;
begin
  s := TEdit(Sender).Text;
  C3RequestData[TEdit(Sender).TabOrder+2] := ASCII27sEG(Ord(s[1]));
  if(TEdit(Sender).Name <> 'edtRateX00001') then SelectNext(ActiveControl, True, True)
  else ActiveControl := edtRateX10000;
  fC3Request := True;
end;

procedure TMainForm.edtCabinAltX10000Change(Sender: TObject);
var s : string;
begin
  s := TEdit(Sender).Text;
  C3RequestData[TEdit(Sender).TabOrder+7] := ASCII27sEG(Ord(s[1]));
  if(TEdit(Sender).Name <> 'edtCabinAltX00001') then SelectNext(ActiveControl, True, True)
  else ActiveControl := edtCabinAltX10000;
  fC3Request := True;
end;


procedure TMainForm.edtDifPressX100Change(Sender: TObject);
var s : string;
begin
  s := TEdit(Sender).Text;
  C3RequestData[TEdit(Sender).TabOrder+12] := ASCII27sEG(Ord(s[1]));
  if(TEdit(Sender).Name <> 'edtDifPressX001') then SelectNext(ActiveControl, True, True)
  else ActiveControl := edtDifPressX100;
  fC3Request := True;
end;

procedure TMainForm.edtDifPressX001Change(Sender: TObject);
var s : string;
begin
  if Tshape(decimalPoint).Tag = 1 then
    begin
      s := TEdit(Sender).Text;
      //C3RequestData[TEdit(Sender).TabOrder+12] := ASCII27sEG(Ord(s[1]) + byte(10000000));
      C3RequestData[TEdit(Sender).TabOrder+12] := ASCII27sEG(Ord(s[1]) ) or $80;
    end
  else if Tshape(decimalPoint).Tag = 0 then
    begin
      s := TEdit(Sender).Text;
      C3RequestData[TEdit(Sender).TabOrder+12] := ASCII27sEG(Ord(s[1]));
    end;
  if(TEdit(Sender).Name <> 'edtDifPressX001') then SelectNext(ActiveControl, True, True)
  else ActiveControl := edtDifPressX100;
  fC3Request := True;
end;

procedure TMainForm.edtLDEConstAltX10000Change(Sender: TObject);
var s : string;
begin
  s := TEdit(Sender).Text;
  C3RequestData[TEdit(Sender).TabOrder+15] := ASCII27sEG(Ord(s[1]));
  if(TEdit(Sender).Name <> 'edtLDEConstAltX00001') then SelectNext(ActiveControl, True, True)
  else ActiveControl := edtLDEConstAltX10000;
  fC3Request := True;
end;

procedure TMainForm.edtRateX10000Click(Sender: TObject);
begin
  TEdit(Sender).Selectall;
end;

procedure TMainForm.edtCabinAltX10000Click(Sender: TObject);
begin
  TEdit(Sender).Selectall;
end;

procedure TMainForm.edtDifPressX100Click(Sender: TObject);
begin
  TEdit(Sender).Selectall;
end;

procedure TMainForm.edtLDEConstAltX10000Click(Sender: TObject);
begin
  TEdit(Sender).Selectall;
end;

procedure TMainForm.edtIPAddressKeyPress(Sender: TObject; var Key: char);
var SimtekIni : TIniFile;
begin
  case key of
   '0'..'9':;
   '.':;
  #8:;
  #13:begin
      with TEdit(Sender) do
        begin
        SimtekIni                 := TIniFile.Create('C:\Simtek.Ini');
        with SimtekIni do begin   WriteString(InstSec, InstSecK01, edtIPAddress.Text); end; {Set the default flags for startup          }
        SimtekIni.Free;
        end;// with
      MainForm.Caption            := buildMainCaption;       //
      end//case #13
   else Key := #00;
   end;
end;

procedure TMainForm.mm01s01Click(Sender: TObject);
begin
  Close;
end;

procedure TMainForm.mm02s01Click(Sender: TObject);
begin
  TRichEdit(Sender).Clear;                              // blank the window
end;

procedure TMainForm.mm02s03Click(Sender: TObject);
begin
  NumberOfRequests                := 0;
  NumberOfResponse                := 0;
end;

procedure TMainForm.mm03s01Click(Sender: TObject);
var i : byte;
begin
  mm03s02.Checked           := mm03s01.Checked;
  mm03s01.Checked           := not mm03s01.Checked;
  gRate.ui                  := 0;
  gRate.s                   := '';
  gRate.ai                  := 0;
  gRate.us                  := 0;
  for i := 0 to 255 do
    gRate.ar[i]             := 0;
  gRate.ui                  := 0;
  gRate.uc                  := 0;
  gRateC1                   := gRate;
  gRateC2                   := gRate;
  gRateC3                   := gRate;
  gRateC4                   := gRate;
end;

procedure TMainForm.mm03s03Click(Sender: TObject);
begin
  mm03s03.Checked                 := mm03s04.Checked;
  mm03s04.Checked                 := not mm03s04.Checked;
end;

procedure TMainForm.mm03s05s00Click(Sender: TObject);
begin
  TMenuItem(Sender).Checked       := not TMenuItem(Sender).Checked;
end;

procedure TMainForm.mm04Click(Sender: TObject);
var i : integer;
begin
  mm04.Checked                    := not mm04.Checked;
  fScrollReq                      := mm04.Checked;
  if mm04.Checked then
    begin
    gRateScroll.ui                  := 0;
    gRateScroll.s                   := '';
    gRateScroll.ai                  := 0;
    gRateScroll.us                  := 0;
    for i := 0 to 255 do
      gRateScroll.ar[i]             := 0;
    gRateScroll.uc                  := 0;
    end
  else
    begin
    end;
  dScrollValue                    := 1;
  ScrollCount                     := ScrollCount;
  fImageRedraw                    := True;
  fC2Request                      := True;
end;

procedure TMainForm.mm05Click(Sender: TObject);
var UserTransmitString,CharacterCreationString,StringToTransmit : string;
    CharacterPointer,CharacterToTransmit,StringLength : byte;
    x : integer;
begin
  BurstFileContents := TStringList.Create;
  BurstFileContents.Clear;
  try
    BurstFileContents.LoadFromFile(BurstFileName);
  except
    mm05.Visible := False;
  end;
  x := 0;
  try
    while(x < BurstFileContents.Count)do
      begin
      UserTransmitString := BurstFileContents.Strings[x];

      StringLength := Length(UserTransmitString);
      CharacterPointer := 1;
      StringToTransmit := '';
      While CharacterPointer <= StringLength do
        begin
        CharacterCreationString := '$' + UserTransmitString[CharacterPointer] + UserTransmitString[CharacterPointer+1];
        CharacterToTransmit := StrToInt(CharacterCreationString);
        StringToTransmit := StringToTransmit + Chr(CharacterToTransmit);
        CharacterPointer := CharacterPointer + 2;
        end;
      if StringToTransmit <> '' then cpOutputData(StringToTransmit, False);
      inc(x);
      end;
  except
    MessageDlg('Error Illegal Character In Source File',mtError,[mbOk],0);
  end;//try
  BurstFileContents.Free;
end;

procedure TMainForm.mm06Click(Sender: TObject);
begin
  fC0Request                      := True;
end;

procedure TMainForm.mm07Click(Sender: TObject);
begin
  fC1Request                      := True;
end;

procedure TMainForm.mm08Click(Sender: TObject);
begin
  fC2Request                      := True;
end;

procedure TMainForm.mm09Click(Sender: TObject);
begin
  fC3Request                      := True;
end;

procedure TMainForm.mm10Click(Sender: TObject);
begin
  fC4Request                      := True;
end;

procedure TMainForm.mm11Click(Sender: TObject);
begin
  fC5Request                      := True;
end;

procedure TMainForm.mm12Click(Sender: TObject);
begin
  fC6Request                      := True;
end;

procedure TMainForm.responseReset;
begin
    TrafficLightsOff;                          // Turn off the communications status indicators
    memoTx.Clear;                              // Blank the dataout and datain
    memoRx.Clear;                              // Blank the dataout and datain

    mm06.Caption                     := '&'+C0Command;
//    mm07.Caption                     := '&'+C1Command;
    mm07.Visible                     := False;
    mm08.Caption                     := C2Command;
    mm09.Caption                     := C3Command;
//    mm10.Caption                     := C4Command;
    mm10.Visible                     := False;
    mm11.Caption                     := C5Command;
    mm12.Caption                     := C6Command;
//    mm13.Caption                     := C7Command;
    mm13.Visible                     := False;
//    mm14.Caption                     := C8Command;
    mm14.Visible                     := False;

//    mm03s05s01.Caption               := '&'+C1Command;
    mm03s05s02.Caption               := '&'+C2Command;
    mm03s05s03.Caption               := '&'+C3Command;
    mm03s05s04.Caption               := '&'+C4Command;
    mm03s05s05.Caption               := '&'+C5Command;
    mm03s05s06.Caption               := '&'+C6Command;
    mm13.Visible                     := False;
    mm14.Visible                     := False;

    lblFirmwareValue1.Caption        := C2DeviceValDefault;
    lblFirmwareValue2.Caption        := C2DeviceValDefault;
    lblFirmwareValue3.Caption        := C2DeviceValDefault;

    tkbDimming1Old                   := 255;
    tkbDimming1.Position             := 0;
    lblDimming1.Caption              := lblDimming1Cap + '0';
    lblDimming1.Hint                 := lblDimming1Hint;
    tkbDimming2Old                   := 255;
    tkbDimming2.Position             := 0;
    lblDimming2.Caption              := lblDimming2Cap + '0';
    lblDimming2.Hint                 := lblDimming2Hint;
//    tkbDimming3Old                   := 255;
//    tkbDimming3.Position             := 0;
//    lblDimming3.Caption              := lblDimming3Cap + '0';
//    lblDimming3.Hint                 := lblDimming3Hint;

    MainForm.Caption                 := buildMainCaption;       //
    lblTransmitRate.Caption          := TSCapHeader + '0.00Hz';
//    lblC1TransmitRate.Caption        := C1CapHeader + '0.00Hz';
    lblC2TransmitRate.Caption        := C2CapHeader + '0.00Hz';
    lblC3TransmitRate.Caption        := C3CapHeader + '0.00Hz';
//    lblC4TransmitRate.Caption        := C4CapHeader + '0.00Hz';
    lblTestSoftwarePNValue.Caption   := lblTestSoftwareCap;
    lblTestSoftwareRevValue.Caption  := lblTestSoftwareRev;
    lblWARNING.Caption               := lblWARNINGCaption;
//    ActiveControl                    := UserImage;
end;

procedure TMainForm.responseStatus(ptrCK: pbyte);
var tmp : byte;
begin
  inc(NumberOfResponse);
  inc(ptrCK);                    // Command byte
///////////////////// first byte of status command //////////////////////
  swValveControl.Tag := (ptrCK^ and $03);                                       // hex value of byte info for the state of each switch
  inc(ptrCK);                                                                   // check the 8664 paper for more info (C. Mason helped with this)
/////////////////////  second byte of status command ////////////////////
  potAutoRate.Tag := (ptrCK^);                                                  // doesn't need a mask because it's the entire byte
  inc (ptrCK);
/////////////////////  third byte of status command /////////////////////
  swSW01.Tag := (ptrCK^ and $03);
  encoderPressed := (ptrCK^ and $10);
  inc (ptrCK);
/////////////////////  fourth byte of status command ////////////////////
  knobValveControl.Tag:= (ptrCK^ and $3f);
  inc (ptrCK);
/////////////////////  fifth byte of status command /////////////////////
  encoderLDEConstAlt.Tag:= (ptrCK^);
  fImageRedraw    := True;
end;

procedure TMainForm.responseFirmware(s : string);
var
  si    : string;
  b1,b2 : byte;
begin
    inc(NumberOfResponse);
    b2 := Ord(s[2]);
    b1 := ((b2 shr 4) and $0F) or $30;
    b2 := (b2 and $0F) or $30;
    si := ': ' + chr(b1) + chr(b2) + '-';
    b2 := Ord(s[3]);
    b1 := ((b2 shr 4) and $0F) or $30;
    b2 := (b2 and $0F) or $30;
    si := si + chr(b1) + chr(b2);
    b2 := Ord(s[4]);
    b1 := ((b2 shr 4) and $0F) or $30;
    b2 := (b2 and $0F) or $30;
    si := si + chr(b1) + chr(b2) + ' rev ';
    si := si + s[5];
    lblFirmwareValue1.Caption := si;
    b2 := Ord(s[6]);
    b1 := ((b2 shr 4) and $0F) or $30;
    b2 := (b2 and $0F) or $30;
    si := ': ' + chr(b1) + chr(b2) + '-';
    b2 := Ord(s[7]);
    b1 := ((b2 shr 4) and $0F) or $30;
    b2 := (b2 and $0F) or $30;
    si := si + chr(b1) + chr(b2);
    b2 := Ord(s[8]);
    b1 := ((b2 shr 4) and $0F) or $30;
    b2 := (b2 and $0F) or $30;
    si := si + chr(b1) + chr(b2) + ' rev ';
    si := si + s[9];
    lblFirmwareValue2.Caption := si;
    b2 := Ord(s[10]);
    b1 := ((b2 shr 4) and $0F) or $30;
    b2 := (b2 and $0F) or $30;
    si := ': ' + chr(b1) + chr(b2) + '-';
    b2 := Ord(s[11]);
    b1 := ((b2 shr 4) and $0F) or $30;
    b2 := (b2 and $0F) or $30;
    si := si + chr(b1) + chr(b2);
    b2 := Ord(s[12]);
    b1 := ((b2 shr 4) and $0F) or $30;
    b2 := (b2 and $0F) or $30;
    si := si + chr(b1) + chr(b2) + ' rev ';
    si := si + s[13];
    lblFirmwareValue3.Caption := si;
end;

procedure TMainForm.memoRevisionEnter(Sender: TObject);
begin
  memoRevision.Height            := 290;
end;

procedure TMainForm.memoRevisionExit(Sender: TObject);
begin
  memoRevision.Height     := 38;
  ActiveControl           := nil;
end;

procedure TMainForm.memoRevisionMouseLeave(Sender: TObject);
begin
  memoRevision.Height     := 38;
  ActiveControl           := nil;
end;

procedure TMainForm.PopupMenu1Popup(Sender: TObject);
var   ln,ch : integer;
const copy   = 0;
      cut    = 1;
      paste  = 2;
      delete = 3;
      select = 5;
      clear  = 7;
      print  = 9;
begin
  ln        := 0;
  ch        := 0;
  if PopupMenu1.PopupComponent = memoTx then
   begin
   ln       := memoTx.Sellength;
   ch       := length(memoTx.Text);
   end
  else if PopupMenu1.PopupComponent = memoRx then
   begin
   ln       := memoRx.Sellength;
   ch       := length(memoRx.Text);
   end
  else
   Beep;
  if ln > 0 then
   begin
   PopUpMenu1.Items[copy].Enabled   := True;
   PopUpMenu1.Items[cut].Enabled    := True;
   PopUpMenu1.Items[delete].Enabled := True;
   end
  else
   begin
   PopUpMenu1.Items[copy].Enabled   := False;
   PopUpMenu1.Items[cut].Enabled    := False;
   PopUpMenu1.Items[delete].Enabled := False;
   end;
  if ch <> 0 then
   begin
   PopUpMenu1.Items[select].Enabled := True;
   PopUpMenu1.Items[clear].Enabled  := True;
   PopUpMenu1.Items[print].Enabled  := True;
   end
  else
   begin
   PopUpMenu1.Items[select].Enabled := False;
   PopUpMenu1.Items[clear].Enabled  := False;
   PopUpMenu1.Items[print].Enabled  := False;
   end;
end;

procedure TMainForm.tbUpdateRateChange(Sender: TObject);
begin
 if tbUpdateRate.Position = 0 then DXTimer1.Interval := 0 else DXTimer1.Interval := 1;
 if tkbScrollRate.Position < tbUpdateRate.Position then
   tkbScrollRate.Position := tbUpdateRate.Position;
end;



procedure TMainForm.btnLeftAutoRateMouseUp(Sender: TObject;
  Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
var pos  : integer;
  const moveALittle = 5;
  maxPotNum = 255;
  minPotNum = 0;

begin


  if potAutoRate.Tag >= maxPotNum then
    begin
      pos := (potAutoRate.Tag - moveALittle);
    end
  else if potAutoRate.Tag <= minPotNum then
    begin
      pos := minPotNum;
    end
  else
    begin
      pos := (potAutoRate.Tag - moveALittle);
    end;

  potAutoRate.Tag := pos;                                                       //
  potAutoRateIndex.Visible := True;
  fImageRedraw := True;

end;

procedure TMainForm.btnRightAutoRateMouseUp(Sender: TObject;
  Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
var pos  : integer;
  const moveALittle = 5;
  maxPotNum = 255;
  minPotNum = 0;

begin

  if potAutoRate.Tag >= maxPotNum then
    begin
      pos := maxPotNum;
    end
  else if potAutoRate.Tag <= minPotNum then
    begin
      pos := potAutoRate.Tag + moveALittle;
    end
  else
    begin
      pos := potAutoRate.Tag + moveALittle;
    end;

  potAutoRate.Tag := pos;                                                       //
  potAutoRateIndex.Visible := True;
  fImageRedraw := True;
end;

procedure TMainForm.btnLeftLDEConstAltMouseUp(Sender: TObject;
  Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
  var pos  : integer;
  const moveALittle = 5;
  maxEncoderNum = 255;
  minEncoderNum = 0;

begin

  if encoderLDEConstAlt.Tag > maxEncoderNum then
    begin
      encoderLDEConstAlt.Tag := minEncoderNum;
      pos := encoderLDEConstAlt.Tag - moveALittle;
    end
  else if encoderLDEConstAlt.Tag < minEncoderNum then
    begin
      encoderLDEConstAlt.Tag := maxEncoderNum;
      pos := encoderLDEConstAlt.Tag - moveALittle;
    end
  else
    begin
      pos := encoderLDEConstAlt.Tag - moveALittle;
    end;

  encoderLDEConstAlt.Tag := pos;                                                //
  encoderLDEConstAltIndex.Visible := True;
  fImageRedraw := True;

end;




procedure TMainForm.btnRightLDEConstAltMouseUp(Sender: TObject;
  Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
  var pos  : integer;
  const moveALittle = 5;
  maxEncoderNum = 255;
  minEncoderNum = 0;

begin
  if encoderLDEConstAlt.Tag > maxEncoderNum then
  begin
    encoderLDEConstAlt.Tag := minEncoderNum;
    pos := encoderLDEConstAlt.Tag + moveALittle;
  end
  else if encoderLDEConstAlt.Tag < minEncoderNum then
  begin

    encoderLDEConstAlt.Tag := maxEncoderNum;
    pos := encoderLDEConstAlt.Tag + moveALittle;
  end
  else
  begin
    pos := encoderLDEConstAlt.Tag + moveALittle;
  end;

  encoderLDEConstAlt.Tag := pos;                                                //
  encoderLDEConstAltIndex.Visible := True;
  fImageRedraw := True;
end;

function TMainForm.calculateRate(ct : double;g : Tx;rc : boolean;l : TObject;t : TObject; s : string):Tx;
var
   i   : integer;
   cus : double; // current update speed
   cuh : double; // current update Hz
   cua : double; // current update average
begin
  if (g.us <> 0) and rc then
    begin
    cus := (ct - g.us) * 86400;
    if cus <> 0 then
      begin
      g.ar[g.ai] := cus;
      inc(g.ai);
      if g.ui < 255 then Inc(g.ui);
      cua := 0;
      for i := 0 to (g.ui - 1) do
        begin  cua := cua + g.ar[i];    end;
      cus := cua / g.ui;
      cuh := RoundTo((1 / cus),-2);
      g.s := FloatToStr(cuh) +'Hz';
      g.us := ct;
      end;//if cus <> 0 then
    end// if g.us <> 0 then
  else if rc then
    begin
    g.us := ct;
    g.ai := 0;
    end;//else if g.us <> 0 then
  TLabel(l).Caption := s + g.s + ' ' +  IntToStr(TTrackBar(t).Position);
  Result := g;
end;

procedure TMainForm.DXTimer1Timer(Sender: TObject; LagCount: Integer);
const numSwitches = 7;                                                          // number of switches of a single type (not going to be used unless we are giving feedback to switches)
      numIndicators = 1;                                                         // One "Indicator" and it is the decimal point
      numDisplays = 18;                                                          // number of displays, should also be of a single type
      maxNumDisplayed = 9;

var  rxtimeout : integer;                 // timeout time for attempt to read receive buffer
     wdlist    : array[0..10] of string;  //
     wrstr     : string;                  // string to write to transmit buffer
     li        : integer;                 // line index
     counter   : integer;                 //
     ratecalc  : boolean;                 //
     ctime     : double;                  //

     arraySwitches   : array[1..numSwitches]   of TShape;                       // My switches are all shapes                         ////////////////////////////////////////////////////////////////
     arrayIndicators : array[1..numIndicators] of TShape;                     // No indicators on this instrument                   //  Each of these arrays will hold the names of their objects //
     arrayDisplays   : array[1..numDisplays]   of TEdit;                        // The Displays are Edit boxes                        ////////////////////////////////////////////////////////////////
     J : byte;                                                                  // counting through for loop
     K : byte;                                                                  // counting through for loop
     chars     : string;
begin
  chars := ' 0123456789abcdefg ';
  rxtimeout                   := tkbRecieveTimeOut.Position;
  wrstr                       := '';
  lblRequestsSent.Caption     := ': ' + IntToStr(NumberOfRequests);
  lblResponseRecieved.Caption := ': ' + IntToStr(NumberOfResponse);

  if NumberOfRequests > NumberOfResponse then
    begin
    wrstr     := IdUDPClient1.ReceiveString(rxtimeout);
    if wrstr <> '' then decodeRecievedData(wrstr);
    RXCount    := 30;
    setColorCOMLightRX(clLime,1);
    end;// if NumberOfRequests > NumberOfResponse then

  if fScrollReq then
    begin
    if ScrollCount > tkbScrollRate.Position then
      begin
      li := length(chars) - 1;

      arrayIndicators[1] := decimalPoint;

      arrayDisplays[1] := edtRateX10000;
      arrayDisplays[2] := edtRateX01000;
      arrayDisplays[3] := edtRateX00100;
      arrayDisplays[4] := edtRateX00010;
      arrayDisplays[5] := edtRateX00001;

      arrayDisplays[6]  := edtCabinAltX10000;
      arrayDisplays[7]  := edtCabinAltX01000;
      arrayDisplays[8]  := edtCabinAltX00100;
      arrayDisplays[9]  := edtCabinAltX00010;
      arrayDisplays[10] := edtCabinAltX00001;

      arrayDisplays[11] := edtDifPressX100;
      arrayDisplays[12] := edtDifPressX010;
      arrayDisplays[13] := edtDifPressX001;

      arrayDisplays[14] := edtLDEConstAltx10000;
      arrayDisplays[15] := edtLDEConstAltx01000;
      arrayDisplays[16] := edtLDEConstAltx00100;
      arrayDisplays[17] := edtLDEConstAltx00010;
      arrayDisplays[18] := edtLDEConstAltx00001;

////////////////////////////////////////////////////////////////////////////////
///  Scroll through the indicator and display                                ///
////////////////////////////////////////////////////////////////////////////////
      for J := 1 to numIndicators do
        Tshape(arrayIndicators[J]).OnMouseUp(arrayIndicators[J], mbLeft, [], 0,0);   // loops through indicator buttons
      for K := 1 to numDisplays do
        arrayDisplays[K].Text := chars[dScrollValue];
//      for K := 1 to numDisplays do
//        begin
//        if arrayDisplays[K].Text = inttostr(maxNumDisplayed) then             // checks to see if the Display on the box is at the
//          begin                                                               // max number we want it to be (int to string to ensure data plays nicely)
//          arrayDisplays[K].Text := '0';
//          end
//        else
//          begin
//          arrayDisplays[K].Text :=                                            // increment count on displays if not at max number
//          inttostr(strtoint(arrayDisplays[K].Text) + 1);                      // data conversion for math and then placing into .Text
//          end
//        end;
      gRateScroll := calculateRate(ctime,       gRateScroll,    True,  lblScrollRate, tkbScrollRate, ScrollCapHeader);
      if gRateScroll.uc >= (tkbC6UpdateRate.Position) then begin gRateScroll.uc := 0; end;
      if dScrollValue > li then dScrollValue := 1;
      fImageRedraw    := True;
      ScrollCount     := 255;
      inc(dScrollValue);
      end;//if DispScrolRateCnt > tkbScrollRate.Position then
    inc(ScrollCount);
    end;
  wrstr           := '';
  mm03s05.Checked := mm03s05s01.Checked
                  or mm03s05s02.Checked
                  or mm03s05s03.Checked
                  or mm03s05s04.Checked
                  or mm03s05s05.Checked
                  or mm03s05s06.Checked;
  inc(gRate.uc);

  if gRateC1.uc >= (tkbC1UpdateRate.Position) then
    begin
    if mm03s05.Checked and mm03s02.Checked then
      begin
      if mm03s05s01.Checked then fC1Request := True;
      end;
    gRateC1.uc := 0;
    end; //if gRateC1.uc >= (tkbC1UpdateRate.Position)then

  if gRateC2.uc >= (tkbC2UpdateRate.Position) then
    begin
    if mm03s05.Checked and mm03s02.Checked then
      begin      if mm03s05s02.Checked then fC2Request := True;   end;
    gRateC2.uc := 0;
    end; //if gRateC2.uc >= (tkbC2UpdateRate.Position)then

  if gRateC3.uc >= (tkbC3UpdateRate.Position) then
    begin
    if mm03s05.Checked and mm03s02.Checked then
      begin      if mm03s05s03.Checked then fC3Request := True;   end;
    gRateC3.uc := 0;
    end; //if gRateC3.uc >= (tkbC3UpdateRate.Position)then

  if gRateC4.uc >= (tkbC4UpdateRate.Position) then
    begin
    if mm03s05.Checked and mm03s02.Checked then
      begin      if mm03s05s04.Checked then fC4Request := True;   end;
    gRateC4.uc := 0;
    end; //if gRateC4.uc >= (tkbC4UpdateRate.Position)then

  if gRateC5.uc >= (tkbC5UpdateRate.Position) then
    begin
    if mm03s05.Checked and mm03s02.Checked then
      begin      if mm03s05s05.Checked then fC5Request := True;   end;
    gRateC5.uc := 0;
    end; //if gRateC5.uc >= (tkbC4UpdateRate.Position)then

  if gRateC6.uc >= (tkbC6UpdateRate.Position) then
    begin
    if mm03s05.Checked and mm03s02.Checked then
      begin      if mm03s05s06.Checked then fC6Request := True;   end;
    gRateC6.uc := 0;
    end; //if gRateC6.uc >= (tkbC4UpdateRate.Position)then



  ctime           := Now;
  li              := 0;
  if fC0Request then
    begin
    wrstr         := wrstr + encodeCommandData(C0Request);
    if mm03s04.Checked then
      begin
      inc(li);
      wdlist[li]  := wrstr;
      wrstr       := '';
      end;// mm03s04.Checked
    fC0Request    := False;
    end;// if fC0Request

{  if fC1Request then
    begin
    wrstr         := wrstr + encodeCommandData(C1Request);
    if mm03s04.Checked then
      begin
      inc(li);
      wdlist[li]  := wrstr;
      wrstr       := '';
      end;// mm03s04.Checked
    gRateC1 := calculateRate(ctime, gRateC1, True, lblC1TransmitRate, tkbC1UpdateRate, C1CapHeader);
    fC1Request    := False;
    end;// if fC1Request
 }
  if fC2Request then
    begin
    wrstr         := wrstr + encodeCommandData(C2Request);
    if mm03s04.Checked then
      begin
      inc(li);
      wdlist[li]  := wrstr;
      wrstr       := '';
      end;// mm03s04.Checked
    gRateC2 := calculateRate(ctime, gRateC2, True, lblC2TransmitRate, tkbC2UpdateRate, C2CapHeader);
    fC2Request    := False;
    end;// if fC2Request

  if fC3Request then
    begin
    wrstr         := wrstr + encodeCommandData(C3Request);
    if mm03s04.Checked then
      begin
      inc(li);
      wdlist[li]  := wrstr;
      wrstr       := '';
      end;// mm03s04.Checked
    gRateC3 := calculateRate(ctime, gRateC3, True, lblC3TransmitRate, tkbC3UpdateRate, C3CapHeader);
    fC3Request    := False;
    end;// if fC3Request

  if fC4Request then
    begin
    wrstr         := wrstr + encodeCommandData(C4Request);
    if mm03s04.Checked then
      begin
      inc(li);
      wdlist[li]  := wrstr;
      wrstr       := '';
      end;// mm03s04.Checked
    gRateC4 := calculateRate(ctime, gRateC4, True, lblC4TransmitRate, tkbC4UpdateRate, C4CapHeader);
    fC4Request    := False;
    end;// if fC4Request

  if fC5Request then
    begin
    wrstr         := wrstr + encodeCommandData(C5Request);
    if mm03s04.Checked then
      begin
      inc(li);
      wdlist[li]  := wrstr;
      wrstr       := '';
      end;// mm03s04.Checked
    gRateC5 := calculateRate(ctime, gRateC5, True, lblC5TransmitRate, tkbC5UpdateRate, C5CapHeader);
    fC5Request    := False;
    end;// if fC5Request

  if fC6Request then
    begin
    wrstr         := wrstr + encodeCommandData(C6Request);
    if mm03s04.Checked then
      begin
      inc(li);
      wdlist[li]  := wrstr;
      wrstr       := '';
      end;// mm03s04.Checked
    gRateC6 := calculateRate(ctime, gRateC6, True, lblC6TransmitRate, tkbC6UpdateRate, C6CapHeader);
    fC6Request    := False;
    end;// if fC6Request

  if mm03s03.Checked and (wrstr <> '') then
    begin
    inc(li);
    wdlist[li]    := wrstr;
    wrstr         := '';
    end;//if mm03s01.Checked

  if gRate.uc >= (tbUpdateRate.Position) then
    begin
    if mm03s02.Checked then gRate := calculateRate(ctime,    gRate, True, lblTransmitRate, tbUpdateRate, TSCapHeader)
    else                    gRate := calculateRate(gRate.uc, gRate, True, lblTransmitRate, tbUpdateRate, TSCapHeader);

    if li <> 0 then
      begin
      for counter  := 1 to li do
        begin
        wrstr           := wdlist[counter];
        wdlist[counter] := '';
        if counter       = li then ratecalc := True else  ratecalc := False;
        cpOutputData(wrstr, ratecalc);
        end;//for counter := 1 to li do
      end;//if li <> 0 then
    gRate.uc := 0;
    inc(gRateC1.uc);
    inc(gRateC2.uc);
    inc(gRateC3.uc);
    inc(gRateC4.uc);
    inc(gRateC5.uc);
    inc(gRateC6.uc);
    end;//if gRate.uc >= (tbUpdateRate.Position) then

  if (fImageRedraw and cbxGraphicsEnable.Checked) then
    begin
    paintGUI;
    end;

  if TXCount > 1 then Dec(TXCount);
  if TXCount = 1 then
    begin
    TXCount := 0;
    setColorCOMLightTX(clRed,1);
    end;

  if RXCount > 1 then Dec(RXCount);
  if RXCount = 1 then
    begin
    RXCount := 0;
    setColorCOMLightRX(clRed,1);
    end;
end;

procedure TMainForm.decimalPointMouseUp(Sender: TObject; Button: TMouseButton;  //
  Shift: TShiftState; X, Y: Integer);
  var s : string;                                                               //
begin                                                                           //
  if TShape(Sender).Tag = 0 then                                                // Checks tag of decimal point
    begin                                                                       // toggle decimal point on and off
      TShape(Sender).Tag := 1;                                                  //
      TShape(Sender).Brush.Color := clLime;                                     //
      s := edtDifPressX001.Text;                                                //
      C3RequestData[TEdit(Sender).TabOrder+15] := ASCII27sEG(Ord(s[1]) ) or $80;//
    end                                                                         //
  else if TShape(Sender).Tag = 1 then                                           //
    begin                                                                       //
      TShape(Sender).Tag := 0;                                                  //
      TShape(Sender).Brush.Color := clBlack;                                    //
      s := edtDifPressX001.Text;                                                //
      C3RequestData[TEdit(Sender).TabOrder+15] := ASCII27sEG(Ord(s[1]));        //
                                                                                //
    end;                                                                        //
    fC3Request := True;                                                         //
end;                                                                            //

procedure TMainForm.decodeRecievedData(s: string);
var
  SMsg      : string;
  lsLineNum : string;
  i         : integer;
  count     : byte;
  cal,giv   : byte;
  DPtr      : pbyte;
  SPtr      : pbyte;
begin
  if length(s) <> 0 then
    begin
    i := 1;
    while i <= length(s) do
      begin
      case Ord(s[i]) of
//  C2Response  = C2Request;   C2Responselength  = 9;  // program response
//  C6Response  = C6Request;   C6Responselength  = 6;  // status
        C2Response : begin  // program revision
          SPtr     := Addr(s[i]);
          DPtr     := Addr(C2ResponseData[1]);
          CopyMemory(DPtr, SPtr, (C2Responselength - 1 ));
          responseFirmware(s);
          i        := i + (C2Responselength - 1 );
          fImageRedraw := True;
          end;//case = C2Response
        C6Response : begin  // switch status
          SPtr     := Addr(s[i]);
          DPtr     := Addr(C6ResponseData[1]);
          CopyMemory(DPtr, SPtr, C6Responselength);
          responseStatus(DPtr);
          i        := i + C6Responselength;
          fImageRedraw := True;
          end;//case = C1Response
        else
          i := i + 1;
        end;// case Ord(s[i]) of
      end;// while i <= length(s) do
    end;// if length(s) <> 0 then

  SMsg :='';
  if cbxRDXWindowEnable.Checked then
    begin
    count := length(s);
    SMsg  := '';
    for i := 1 to count do {Loop through the receive buffer and}
      begin
      SMsg  := SMsg + IntToHex(ord(s[i]),2)+' '; {Convert the bytes to a pascal string}
      end;
    if memoRx.Lines.Count >= RxWinLimit then
      memoRx.Clear;
    lsLineNum := 'RX '+ IntToStr(memoRx.Lines.Count) + ' -> ';
    While length(lsLineNum) < (RxPlace + 7) do
      Insert('0',lsLineNum,4);
    memoRx.Lines.Add(lsLineNum + SMsg);          {then store them in the TX Window }
    lblReceived.Caption := 'Lines Received : ' + IntToStr(memoRx.Lines.Count);
    end;
end;

procedure TMainForm.calculateRotaryIndexPos(rot, indx: TObject;stops: integer);

var pos  : integer;
    ang  : double;
    xpnt : double;
    ypnt : double;
    rrad : integer;
    ccX  : extended;
    ccY  : extended;


begin
  pos := TShape(rot).Tag;
  case pos of
    $01 : pos := 7;
    $02 : pos := 0;
    $04 : pos := 1;
    $08 : pos := 2;
    $10 : pos := 3;
    //$20 : pos := 4;                                                           // adjustments for orientation and number of stops on rotary switch
    //$40 : pos := 5;                                                           // each number represents a position
    //$80 : pos := 6;                                                           //
    else  pos := 7;                                                             //
    end;
  rrad        := round(TShape(rot).Width / 2) - 7;
  ccX         := TShape(rot).Left + ((TShape(rot).Width  / 2) - 4);
  ccY         := TShape(rot).Top  + ((TShape(rot).Height / 2) - 4);
  ang         := pos * ((2 * PI) / stops) - {adding this for offset}            // this offset is used to rotate starting index
                                              15 * ((2 * PI) / stops);          // 15 is the amount it is being adjusted. play with this as needed
  ang         := ang - PI;                                                                                    //  s2      s3
  xpnt        := (cos(ang) * rrad) + ccX;                                       //tkbC1UpdateRate.Position;   //  86     282
  ypnt        := (sin(ang) * rrad) + ccY;                                       //tkbC2UpdateRate.Position;   // 309     30
  TShape(indx).Top  := integer(Round(ypnt));
  TShape(indx).Left := integer(Round(xpnt));
end;

procedure TMainForm.calculateEncoderIndexPos(rot, indx: TObject;stops: integer);
const encoderMax  = 255;
      encoderMin  = 0;
var pos    : integer;
    ang    : double;
    xpnt   : double;
    ypnt   : double;
    rrad   : integer;
    ccX    : extended;
    ccY    : extended;
begin
  pos := TShape(rot).Tag;

  if pos > encoderMax then
    begin
      pos := encoderMin;
    end
  else if pos < encoderMin then
    begin
      pos := encoderMax;
    end
  else
    begin
    rrad        := round(TShape(rot).Width / 2) - 7;
    ccX         := TShape(rot).Left + ((TShape(rot).Width  / 2) - 4);
    ccY         := TShape(rot).Top  + ((TShape(rot).Height / 2) - 4);
    ang         := pos * ((2 * PI) / stops);
    ang         := ang - PI;                                                                                    //  s2      s3
    xpnt        := (cos(ang) * rrad) + ccX;                                       //tkbC1UpdateRate.Position;   //  86     282
    ypnt        := (sin(ang) * rrad) + ccY;                                       //tkbC2UpdateRate.Position;   // 309     30
    TShape(indx).Top  := integer(Round(ypnt));
    TShape(indx).Left := integer(Round(xpnt));
    // adding this to try and make firmware response work
    encoderLDEConstAltIndex.Tag := pos;
    end;
end;

procedure TMainForm.calculatePotIndexPos(rot, indx: TObject; offset, stops: integer);
const potMax  = 255;
      potMin  = 0;
var ang    : double;
    xpnt   : double;
    ypnt   : double;
    rrad   : integer;
    ccX    : extended;
    ccY    : extended;
    pos    : integer;
begin
  pos := TShape(rot).Tag;

  if pos > potMax then
    begin
      pos := potMin;
    end
  else if pos < potMin then
    begin
      pos := potMax;
    end
  else
    begin
    rrad        := round(TShape(rot).Width / 2) - 7;
    ccX         := TShape(rot).Left + ((TShape(rot).Width  / 2) - 4);
    ccY         := TShape(rot).Top  + ((TShape(rot).Height / 2) - 4);
    ang         := (pos * ((2 * PI) / stops)) - (offset * ((2 * PI) / stops));    // formula +/- offset from 0pi
    ang         := ang - PI;                                                                                    //  s2      s3
    xpnt        := (cos(ang) * rrad) + ccX;                                       //tkbC1UpdateRate.Position;   //  86     282
    ypnt        := (sin(ang) * rrad) + ccY;                                       //tkbC2UpdateRate.Position;   // 309     30
    TShape(indx).Top  := integer(Round(ypnt));
    TShape(indx).Left := integer(Round(xpnt));
    end;
end;

procedure TMainForm.paintGUI;
begin
  fImageRedraw := False;
  setToggleSwitch(swSw01.Tag, 35, swSW01Bat, lblsw01t,      nil, lblsw01b);     // Emergency Depress Toggle Redraw // 35 looked good when the toggle was moved
  setRotarySwitch(knobValveControl,knobValveControlIndex,knobConstAlt,knobMan,knobAuto,knobNoPress,knobAuxVent, 8); // rotary switch redraw
  setEncoder(encoderLDEConstAlt, encoderLDEConstAltIndex, 255);                 // encoder redraw
  setPot(potAutoRate, potAutoRateIndex, 360);                                   // pot redraw
  setRockerSwitch(swValveControl.Tag, swValveControl, shapeOpen, shapeClose, labelOpen, labelClose);  //rocker switch redraw
  setEncoderPress(encoderLDEConstAlt);
end;

procedure TMainForm.lblFirmwareTitle1Click(Sender: TObject);
begin
  lblFirmwareValue1.Caption := C2DeviceValDefault;
  lblFirmwareValue2.Caption := C2DeviceValDefault;
  lblFirmwareValue3.Caption := C2DeviceValDefault;
end;

end.
