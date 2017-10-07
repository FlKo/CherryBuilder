object SymbolHintForm: TSymbolHintForm
  Left = 0
  Top = 0
  BorderStyle = bsNone
  Caption = 'SymbolHintForm'
  ClientHeight = 97
  ClientWidth = 265
  Color = 13158600
  Font.Charset = ANSI_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Segoe UI'
  Font.Style = []
  FormStyle = fsStayOnTop
  KeyPreview = True
  OldCreateOrder = False
  OnKeyDown = FormKeyDown
  OnMouseLeave = FormMouseLeave
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object imgSymbol: TImage
    Left = 38
    Top = 0
    Width = 20
    Height = 20
  end
  object lblKind: TLabel
    Left = 62
    Top = 3
    Width = 4
    Height = 13
    Caption = '-'
    Font.Charset = ANSI_CHARSET
    Font.Color = clBlack
    Font.Height = -11
    Font.Name = 'Segoe UI'
    Font.Style = [fsBold]
    ParentFont = False
  end
  object spDividerTop: TShape
    Left = 58
    Top = 0
    Width = 1
    Height = 20
    Pen.Color = clGray
  end
  object pnlValues: TPanel
    AlignWithMargins = True
    Left = 1
    Top = 20
    Width = 263
    Height = 76
    Margins.Left = 1
    Margins.Top = 20
    Margins.Right = 1
    Margins.Bottom = 1
    Align = alClient
    BevelOuter = bvNone
    Caption = 'pnlValues'
    Color = 15921135
    ParentBackground = False
    ShowCaption = False
    TabOrder = 0
    ExplicitLeft = 62
    ExplicitTop = 39
    object Label1: TLabel
      Left = 1
      Top = 1
      Width = 54
      Height = 13
      Caption = 'Full Name:'
      Font.Charset = ANSI_CHARSET
      Font.Color = clGray
      Font.Height = -11
      Font.Name = 'Segoe UI'
      Font.Style = []
      ParentFont = False
    end
    object Label3: TLabel
      Left = 1
      Top = 17
      Width = 43
      Height = 13
      Caption = 'Returns:'
      Font.Charset = ANSI_CHARSET
      Font.Color = clGray
      Font.Height = -11
      Font.Name = 'Segoe UI'
      Font.Style = []
      ParentFont = False
    end
    object Label4: TLabel
      Left = 1
      Top = 32
      Width = 25
      Height = 13
      Caption = 'Type:'
      Font.Charset = ANSI_CHARSET
      Font.Color = clGray
      Font.Height = -11
      Font.Name = 'Segoe UI'
      Font.Style = []
      ParentFont = False
    end
    object Label5: TLabel
      Left = 1
      Top = 47
      Width = 21
      Height = 13
      Caption = 'File:'
      Font.Charset = ANSI_CHARSET
      Font.Color = clGray
      Font.Height = -11
      Font.Name = 'Segoe UI'
      Font.Style = []
      ParentFont = False
    end
    object Label6: TLabel
      Left = 1
      Top = 62
      Width = 24
      Height = 13
      Caption = 'Line:'
      Font.Charset = ANSI_CHARSET
      Font.Color = clGray
      Font.Height = -11
      Font.Name = 'Segoe UI'
      Font.Style = []
      ParentFont = False
    end
    object lblType: TLabel
      Left = 61
      Top = 32
      Width = 4
      Height = 13
      Caption = '-'
      Font.Charset = ANSI_CHARSET
      Font.Color = clBlack
      Font.Height = -11
      Font.Name = 'Segoe UI'
      Font.Style = []
      ParentFont = False
    end
    object lblFile: TLabel
      Left = 61
      Top = 47
      Width = 4
      Height = 13
      Caption = '-'
      Font.Charset = ANSI_CHARSET
      Font.Color = clBlack
      Font.Height = -11
      Font.Name = 'Segoe UI'
      Font.Style = []
      ParentFont = False
    end
    object lblLine: TLabel
      Left = 61
      Top = 62
      Width = 4
      Height = 13
      Caption = '-'
      Font.Charset = ANSI_CHARSET
      Font.Color = clBlack
      Font.Height = -11
      Font.Name = 'Segoe UI'
      Font.Style = []
      ParentFont = False
    end
    object lblName: TLabel
      Left = 61
      Top = 1
      Width = 4
      Height = 13
      Caption = '-'
      Font.Charset = ANSI_CHARSET
      Font.Color = clBlack
      Font.Height = -11
      Font.Name = 'Segoe UI'
      Font.Style = []
      ParentFont = False
    end
    object lblReturns: TLabel
      Left = 61
      Top = 17
      Width = 4
      Height = 13
      Caption = '-'
      Font.Charset = ANSI_CHARSET
      Font.Color = clBlack
      Font.Height = -11
      Font.Name = 'Segoe UI'
      Font.Style = []
      ParentFont = False
    end
    object spDividerBottom: TShape
      Left = 57
      Top = 0
      Width = 1
      Height = 76
      Pen.Color = clGray
    end
  end
end
