/*
 * Copyright (C) 2013-2014 The CyanogenMod Project
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

package com.android.internal.telephony;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.util.Collections;
import java.util.Map;
import java.util.HashMap;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;

import android.os.Environment;
import android.telephony.Rlog;
import android.util.Xml;

import com.android.internal.util.XmlUtils;

public class Operators{


    // Initialize list of Operator codes
    // this will be taken care of when garbage collection starts.
    private HashMap<String, String>  initList() {
        HashMap<String, String> init = new HashMap<String, String>();
        //taken from spnOveride.java

        FileReader spnReader;

        final File spnFile = new File(Environment.getRootDirectory(),
                                     "etc/selective-spn-conf.xml");

        try {
            spnReader = new FileReader(spnFile);
        } catch (FileNotFoundException e) {
            Rlog.w("Operatorcheck", "Can not open " +
                   Environment.getRootDirectory() + "/etc/selective-spn-conf.xml");
            return init;
        }

        try {
            XmlPullParser parser = Xml.newPullParser();
            parser.setInput(spnReader);

            XmlUtils.beginDocument(parser, "spnOverrides");

            while (true) {
                XmlUtils.nextElement(parser);

                String name = parser.getName();
                if (!"spnOverride".equals(name)) {
                    break;
                }

                String numeric = parser.getAttributeValue(null, "numeric");
                String data    = parser.getAttributeValue(null, "spn");

                init.put(numeric, data);
            }
        } catch (XmlPullParserException e) {
            Rlog.w("Operatorcheck", "Exception in spn-conf parser " + e);
        } catch (IOException e) {
            Rlog.w("Operatorcheck", "Exception in spn-conf parser " + e);
        }
        return init;
    }
    //this will stay persistant in memory when called
    private static String stored = null;
    private static String storedOperators = null;

    public static String operatorReplace(String response){
        // sanity checking if the value is actually not equal to the range apn
        // numerics
        // if it is null, check your ril class.
        if(response == null ||
           (5 != response.length() && response.length() != 6)){
            return response;
        }
        // this will check if the stored value is equal to other.
        // this uses a technique called last known of good value.
        // along with sanity checking
        if(storedOperators != null && stored != null && stored.equals(response)){
            return storedOperators;
        }
        stored = response;
        try {
            // this will find out if it a number then it will catch it based
            // on invalid chars.
            Integer.parseInt(response);
        }  catch(NumberFormatException E){
            // not a number, pass it along to stored operator until the next
            // round.
            storedOperators = response;
            return storedOperators;
        }
        // this code will be taking care of when garbage collection start
        Operators init = new Operators();
        Map<String, String> operators = init.initList();
        storedOperators = operators.containsKey(response) ? operators.get(response) : response;
        return storedOperators;
    }

    // this will not stay persistant in memory, this will be taken care of
    // iin garbage collection routiene.
    private Map<String, String> unOptOperators = null;
    // unoptimized version of operatorreplace for responseOperatorInfos
    // this will provide a little more flexiblilty  in a loop like sisuation
    // same numbers of checks like before
    // this is for the search network functionality
    public String unOptimizedOperatorReplace(String response){
        // sanity checking if the value is actually not equal to the range apn
        // numerics
        // if it is null, check your ril class.
        if(response == null ||
           (5 != response.length() && response.length() != 6)){
            return response;
        }

        try {
            // this will find out if it a number then it will catch it based
            // on invalid chars.
            Integer.parseInt(response);
        }  catch(NumberFormatException E){
            // an illegal char is found i.e a word
            return response;
        }

        if (unOptOperators == null){
            unOptOperators = initList();
        }

        return unOptOperators.containsKey(response) ? unOptOperators.get(response) : response;
    }
}
