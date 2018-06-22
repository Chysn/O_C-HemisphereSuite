class MinMax : public HemisphereApplet {
public:

	void Start() {
		min = 0;
		max = 0;
	}

	void Controller() {
		// Handle CV Inputs
		int cv1 = In(0);
		int cv2 = In(1);

		// Calculate min and max
		min = (cv2 < cv1) ? cv2 : cv1;
		max = (cv2 > cv1) ? cv2 : cv1;

		// Handle CV Outputs: min to 0, max to 1
		Out(0, min);
		Out(1, max);
	}

	void View() {
		gfxPrint(4, 4, "MinMax");
		gfxRect(2, 15, ProportionCV(min, 60), 10);
		gfxRect(2, 30, ProportionCV(max, 60), 10);
	}

	void ScreensaverView() {
		gfxRect(2, 15, ProportionCV(min, 60), 1);
		gfxRect(2, 30, ProportionCV(max, 60), 1);
	}

	void OnButtonPress() {
	}

	void OnButtonLongPress() {
	}

	void OnEncoderMove(int direction) {
	}

private:
	int min;
	int max;

};

////////////////////////////////////////////////////////////////////////////////

MinMax MinMax_instance[2];

void MinMax_Start(int hemisphere) {
	MinMax_instance[hemisphere].SetHemisphere(hemisphere);
	MinMax_instance[hemisphere].Start();
}

void MinMax_Controller(int hemisphere) {
	MinMax_instance[hemisphere].Controller();
}

void MinMax_View(int hemisphere) {
	MinMax_instance[hemisphere].View();
}

void MinMax_Screensaver(int hemisphere) {
	MinMax_instance[hemisphere].ScreensaverView();
}

void MinMax_OnButtonPress(int hemisphere) {
	MinMax_instance[hemisphere].OnButtonPress();
}

void MinMax_OnButtonLongPress(int hemisphere) {
	MinMax_instance[hemisphere].OnButtonLongPress();
}

void MinMax_OnEncoderMove(int hemisphere, int direction) {
	MinMax_instance[hemisphere].OnEncoderMove(direction);
}

