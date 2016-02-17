/*
 * Copyright (C) 2008 The Android Open Source Project
 * Copyright (C) 2016 The Mokee Project
 * Copyright (C) 2016 The CyanogenMod Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.validity.fingerprint;

import android.util.Log;
import java.io.*;
import javax.xml.parsers.*;
import org.xml.sax.*;
import org.xml.sax.helpers.DefaultHandler;

public class ConfigReader {

    public static final String CONFIG_FILE = "/system/etc/ValidityConfig.xml";
    public static final String TAG = "ConfigReader";

    public static class ConfigData {

        public int screenOrientation = 0;
        public int sensorType = 1;
        public String disableButton = "0000";
        public boolean showVideo = false;
        public boolean practiceMode = false;
        public boolean hapticFeedback = true;
        public String fingerActionGenericLabel = "Please swipe";
        public String fingerPlaceOrSwipeLabel = "Place finger";
        public String fingerLiftLabel = "Please lift your finger";
        public SensorBar sensorBar = null;
        public FingerSwipeAnimation fingerSwipeAnimation = null;
        public DisableButtons disableButtons = null;
        public FpDisplay fpDisplay = null;

        ConfigData() {
            sensorBar = new SensorBar();
            fingerSwipeAnimation = new FingerSwipeAnimation();
            fpDisplay = new FpDisplay();
            disableButtons = new DisableButtons();
        }

        public void toStringDebug() {
            Log.i(TAG, "ConfigData():: Screen Orientation=" + screenOrientation + 
                ", SensorType=" + sensorType + ", disabled buttons=" + disableButton + 
                ", fingerPlaceOrSwipeLabel=" + fingerPlaceOrSwipeLabel + 
                ", fingerLiftLabel=" + fingerLiftLabel + 
                ", fingerActionGenericLabel=" + fingerActionGenericLabel + 
                ", showVideo=" + showVideo + ", practiceMode=" + practiceMode + 
                ", hapticFeedback=" + hapticFeedback);
            Log.i(TAG, "Fp Display :: left = " + fpDisplay.xPos + ", top = " + fpDisplay.yPos + 
                ", width = " + fpDisplay.width + ", height = " + fpDisplay.height);
            Log.i(TAG, "DisableButtons =>  home=" + disableButtons.home + ", menu=" + disableButtons.menu + 
                ", search=" + disableButtons.search + ", back=" + disableButtons.back);
            Log.i(TAG, "Finger swipe animation =>  visible=" + fingerSwipeAnimation.visible + 
                ", orientation=" + fingerSwipeAnimation.orientation + ", left=" + fingerSwipeAnimation.xPos + 
                ", top=" + fingerSwipeAnimation.yPos + ", scale width = " + fingerSwipeAnimation.xScale + 
                ", scale height = " + fingerSwipeAnimation.yScale + ", offset length = " + fingerSwipeAnimation.offsetLength + 
                ", outline visible = " + fingerSwipeAnimation.outlineVisible + ", animationSpeed = " + fingerSwipeAnimation.animationSpeed);
            Log.i(TAG, "Fp Display :: left = " + fpDisplay.xPos + ", top = " + fpDisplay.yPos + ", width = " + fpDisplay.width + 
                ", height = " + fpDisplay.height + ", showStartupVideo = " + fpDisplay.showStartupVideo);
        }

        class SensorBar {
            boolean visible = false;
            int xPos = 600;
            int yPos = 800;
            int width = 25;
            int height = 100;
        }

        class FingerSwipeAnimation {
            boolean visible = false;
            int orientation = 0;
            int xPos = 600;
            int yPos = 800;
            float xScale = 1.0F;
            float yScale = 1.0F;
            int offsetLength = 0;
            boolean outlineVisible = true;
            int animationSpeed = 1500;
        }
        public class FpDisplay {
            public int xPos = 0;
            public int yPos = 0;
            public int width = 200;
            public int height = 400;
            public boolean showStartupVideo = false;
        }

        public class DisableButtons {
            public boolean home = false;
            public boolean back = false;
            public boolean menu = false;
            public boolean search = false;
        }

    }


    public static class DataHandler extends DefaultHandler {
    
        private ConfigData _data;
        private boolean _disableButtons;
        private boolean _fingerActionGenericLabel;
        private boolean _fingerLiftLabel;
        private boolean _fingerPlaceOrSwipeLabel;
        private boolean _fingerSwipeAnimation;
        private boolean _fpDisplay;
        private boolean _hapticFeedback;
        private boolean _practiceMode;
        private boolean _screenOrientation;
        private boolean _sensorBar;
        private boolean _sensorType;
        private boolean _showVideo;

        public void characters(char ch[], int start, int length) {
            String chars = new String(ch, start, length);
            chars = chars.trim();
            if (_screenOrientation) {
                _data.screenOrientation = Integer.parseInt(chars);
                return;
            }
            if (_sensorType) {
                _data.sensorType = Integer.parseInt(chars);
                return;
            }
            if (_showVideo) {
                _data.showVideo = Boolean.parseBoolean(chars);
                return;
            }
            if (_practiceMode) {
                _data.practiceMode = Boolean.parseBoolean(chars);
                return;
            }
            if (_hapticFeedback) {
                _data.hapticFeedback = Boolean.parseBoolean(chars);
                return;
            }
            if (_fingerPlaceOrSwipeLabel) {
                _data.fingerPlaceOrSwipeLabel = chars;
                return;
            }
            if (_fingerLiftLabel) {
                _data.fingerLiftLabel = chars;
                return;
            }
            if (_fingerActionGenericLabel) {
                _data.fingerActionGenericLabel = chars;
                return;
            }
            if (_disableButtons) {
                _data.disableButton = chars;
                _data.disableButtons.search = (ch[length - 1] == '1');
                _data.disableButtons.home = (ch[length - 2] == '1');
                _data.disableButtons.back = (ch[length - 3] == '1');
                _data.disableButtons.menu = (ch[length - 4] == '1');
                return;
            }
            if (_sensorBar || _fingerSwipeAnimation || _fpDisplay)
                return;
        }

        public void endDocument() throws SAXException {

        }

        public void endElement(String namespaceURI, String localName, String qName) throws SAXException {
            if (localName.equals("screenOrientation")) {
                _screenOrientation = false;
                return;
            }
            if (localName.equals("sensorType")) {
                _sensorType = false;
                return;
            }
            if (localName.equals("showVideo")) {
                _showVideo = false;
                return;
            }
            if (localName.equals("practiceMode")) {
                _practiceMode = false;
                return;
            }
            if (localName.equals("hapticFeedback")) {
                _hapticFeedback = false;
                return;
            }
            if (localName.equals("fingerPlaceOrSwipeLabel")) {
                _fingerPlaceOrSwipeLabel = false;
                return;
            }
            if (localName.equals("fingerLiftLabel")) {
                _fingerLiftLabel = false;
                return;
            }
            if (localName.equals("fingerActionGenericLabel")) {
                _fingerActionGenericLabel = false;
                return;
            }
            if (localName.equals("disableButtons")) {
                _disableButtons = false;
                return;
            }
            if (localName.equals("sensorBar")) {
                _sensorBar = false;
                return;
            }
            if (localName.equals("fingerSwipeAnimation")) {
                _fingerSwipeAnimation = false;
                return;
            }
            if (localName.equals("fpDisplay")) {
                _fpDisplay = false;
                return;
            }
        }

        public ConfigData getData() {
            return _data;
        }

        public void startDocument() throws SAXException {
            _data = new ConfigData();
        }

        public void startElement(String namespaceURI, String localName, String qName, Attributes atts) throws SAXException {
            if (localName.equals("screenOrientation")) {
                _screenOrientation = true;
                return;
            }
            if (localName.equals("sensorType")) {
                _sensorType = true;
                return;
            }
            if (localName.equals("showVideo")) {
                _showVideo = true;
                return;
            }
            if (localName.equals("practiceMode")) {
                _practiceMode = true;
                return;
            }
            if (localName.equals("hapticFeedback")) {
                _hapticFeedback = true;
                return;
            }
            if (localName.equals("fingerPlaceOrSwipeLabel")) {
                _fingerPlaceOrSwipeLabel = true;
                return;
            }
            if (localName.equals("fingerLiftLabel")) {
                _fingerLiftLabel = true;
                return;
            }
            if (localName.equals("fingerActionGenericLabel")) {
                _fingerActionGenericLabel = true;
                return;
            }
            if (localName.equals("disableButtons")) {
                _disableButtons = true;
                return;
            }
            if (localName.equals("sensorBar")) {
                _sensorBar = true;
                if (atts.getIndex("visible") != -1) {
                    _data.sensorBar.visible = (Integer.parseInt(atts.getValue("visible")) == 1);
                }
                if (atts.getIndex("xPos") != -1)
                    _data.sensorBar.xPos = Integer.parseInt(atts.getValue("xPos"));
                if (atts.getIndex("yPos") != -1)
                    _data.sensorBar.yPos = Integer.parseInt(atts.getValue("yPos"));
                if (atts.getIndex("width") != -1)
                    _data.sensorBar.width = Integer.parseInt(atts.getValue("width"));
                if (atts.getIndex("height") != -1)
                    _data.sensorBar.height = Integer.parseInt(atts.getValue("height"));
                return;
            }
            if (localName.equals("fingerSwipeAnimation")) {
                _fingerSwipeAnimation = true;
                if (atts.getIndex("visible") != -1) {
                    _data.fingerSwipeAnimation.visible = (Integer.parseInt(atts.getValue("visible")) == 1);
                }
                if (atts.getIndex("orientation") != -1)
                    _data.fingerSwipeAnimation.orientation = Integer.parseInt(atts.getValue("orientation"));
                if (atts.getIndex("xPos") != -1)
                    _data.fingerSwipeAnimation.xPos = Integer.parseInt(atts.getValue("xPos"));
                if (atts.getIndex("yPos") != -1)
                    _data.fingerSwipeAnimation.yPos = Integer.parseInt(atts.getValue("yPos"));
                if (atts.getIndex("xScale") != -1)
                    _data.fingerSwipeAnimation.xScale = Float.parseFloat(atts.getValue("xScale"));
                if (atts.getIndex("yScale") != -1)
                    _data.fingerSwipeAnimation.yScale = Float.parseFloat(atts.getValue("yScale"));
                if (atts.getIndex("offsetLength") != -1)
                    _data.fingerSwipeAnimation.offsetLength = Integer.parseInt(atts.getValue("offsetLength"));
                if (atts.getIndex("outlineVisible") != -1)
                    _data.fingerSwipeAnimation.outlineVisible = (Integer.parseInt(atts.getValue("outlineVisible")) == 1);
                if (atts.getIndex("animationSpeed") != -1)
                    _data.fingerSwipeAnimation.animationSpeed = Integer.parseInt(atts.getValue("animationSpeed"));
                return;
            }
            if (localName.equals("fpDisplay")) {
                _fpDisplay = true;
                if (atts.getIndex("xPos") != -1)
                    _data.fpDisplay.xPos = Integer.parseInt(atts.getValue("xPos"));
                if (atts.getIndex("yPos") != -1)
                    _data.fpDisplay.xPos = Integer.parseInt(atts.getValue("yPos"));
                if (atts.getIndex("width") != -1)
                    _data.fpDisplay.width = Integer.parseInt(atts.getValue("width"));
                if (atts.getIndex("height") != -1)
                    _data.fpDisplay.height = Integer.parseInt(atts.getValue("height"));
                if (atts.getIndex("showStartupVideo") != -1) {
                    _data.fpDisplay.showStartupVideo = (Integer.parseInt(atts.getValue("showStartupVideo")) == 1);
                }
            }
        }

    }

    public static ConfigData getData() {
        ConfigData configData = null;
        try {
            InputStream inputStream = new FileInputStream(CONFIG_FILE);
            if (inputStream.available() > 0) {
                DataHandler dataHandler = new DataHandler();
                SAXParserFactory spf = SAXParserFactory.newInstance();
                SAXParser sp = spf.newSAXParser();
                XMLReader xr = sp.getXMLReader();
                xr.setContentHandler((ContentHandler)dataHandler);
                xr.parse(new InputSource(inputStream));
                configData = dataHandler.getData();
            }
            if (inputStream != null) {
                inputStream.close();
            }
            return configData;
        }
        catch (FileNotFoundException fnfe) {
            Log.e("ConfigReader", "Configuration file[/system/etc/ValidityConfig.xml] not found");
        }
        catch (ParserConfigurationException pce) {
            Log.e("ConfigReader", "SAX parse error", pce);
        }
        catch (SAXException se) {
            Log.e("ConfigReader", "SAX error", se);
        }
        catch (IOException ioe) {
            Log.e("ConfigReader", "SAX parse io error", ioe);
        }
        return configData;
    }
}
