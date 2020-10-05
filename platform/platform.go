/*
Copyright (C) 2019-2020 Andreas T Jonsson

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

package platform

type internalPlatform interface{}

type Config func(internalPlatform) error

type VideoMode int

const (
	VideoModeBW40 VideoMode = iota
	VideoModeCO40
	VideoModeBW80
	VideoModeCO80
	VideoModeCGA320
	VideoModeCGA640
)

type AudioSpec struct {
	Freq,
	Channels,
	Samples int
}

type Platform interface {
	HasAudio() bool
	RenderGraphics(vm VideoMode, backBuffer []byte)
	RenderText(mem []byte)
	SetTitle(title string)
	QueueAudio(soundBuffer []byte)
	AudioSpec() AudioSpec
	EnableAudio(b bool)
	SetKeyboardHandler(h func(Scancode))
	SetMouseHandler(h func(byte, int8, int8))
}

var Instance Platform

type Scancode byte

const KeyUpMask Scancode = 0x80

const (
	ScanInvalid Scancode = iota
	ScanEscape
	Scan1
	Scan2
	Scan3
	Scan4
	Scan5
	Scan6
	Scan7
	Scan8
	Scan9
	Scan0
	ScanMinus
	ScanEqual
	ScanBackspace
	ScanTab
	ScanQ
	ScanW
	ScanE
	ScanR
	ScanT
	ScanY
	ScanU
	ScanI
	ScanO
	ScanP
	ScanLBracket
	ScanRBracket
	ScanEnter
	ScanControl
	ScanA
	ScanS
	ScanD
	ScanF
	ScanG
	ScanH
	ScanJ
	ScanK
	ScanL
	ScanSemicolon
	ScanQuote
	ScanBackquote
	ScanLShift
	ScanBackslash
	ScanZ
	ScanX
	ScanC
	ScanV
	ScanB
	ScanN
	ScanM
	ScanComma
	ScanPeriod
	ScanSlash
	ScanRShift
	ScanPrint
	ScanAlt
	ScanSpace
	ScanCapslock
	ScanF1
	ScanF2
	ScanF3
	ScanF4
	ScanF5
	ScanF6
	ScanF7
	ScanF8
	ScanF9
	ScanF10
	ScanNumlock
	ScanScrlock
	ScanKPHome
	ScanKPUp
	ScanKPPageup
	ScanKPMinus
	ScanKPLeft
	ScanKP5
	ScanKPRight
	ScanKPPlus
	ScanKPEnd
	ScanKPDown
	ScanKPPagedown
	ScanKPInsert
	ScanKPDelete
)
