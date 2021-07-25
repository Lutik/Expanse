#pragma once

namespace Expanse::Input
{
	namespace Key
	{
		enum Scancode : uint16_t
		{
            A = 4,
            B = 5,
            C = 6,
            D = 7,
            E = 8,
            F = 9,
            G = 10,
            H = 11,
            I = 12,
            J = 13,
            K = 14,
            L = 15,
            M = 16,
            N = 17,
            O = 18,
            P = 19,
            Q = 20,
            R = 21,
            S = 22,
            T = 23,
            U = 24,
            V = 25,
            W = 26,
            X = 27,
            Y = 28,
            Z = 29,

            Key_1 = 30,
            Key_2 = 31,
            Key_3 = 32,
            Key_4 = 33,
            Key_5 = 34,
            Key_6 = 35,
            Key_7 = 36,
            Key_8 = 37,
            Key_9 = 38,
            Key_0 = 39,

            Return = 40,
            Escape = 41,
            Backspace = 42,
            Tab = 43,
            Space = 44,

            Minus = 45,
            Equals = 46,

            F1 = 58,
            F2 = 59,
            F3 = 60,
            F4 = 61,
            F5 = 62,
            F6 = 63,
            F7 = 64,
            F8 = 65,
            F9 = 66,
            F10 = 67,
            F11 = 68,
            F12 = 69,

            Home = 74,
            PageUp = 75,
            Delete = 76,
            End = 77,
            PageDown = 78,
            Right = 79,
            Left = 80,
            Down = 81,
            Up = 82,

            NumPad_Div = 84,
            NumPad_Mul = 85,
            NumPad_Minus = 86,
            NumPad_Plus = 87,
            NumPad_Enter = 88,
            NumPad_1 = 89,
            NumPad_2 = 90,
            NumPad_3 = 91,
            NumPad_4 = 92,
            NumPad_5 = 93,
            NumPad_6 = 94,
            NumPad_7 = 95,
            NumPad_8 = 96,
            NumPad_9 = 97,
            NumPad_0 = 98,
            NumPad_Period = 99,

            LCtrl = 224,
            LShift = 225,
            LAlt = 226,
            LCmd = 227,
            RCtrl = 228,
            RShift = 229,
            RAlt = 230,
            RCmd = 231,
		};
	}

	namespace MouseButton
	{
		enum MouseButtonIndex : uint8_t
		{
			Left = 1,
			Middle = 2,
			Right = 3,
		};
	}
}