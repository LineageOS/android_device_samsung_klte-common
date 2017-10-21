/*
 * Copyright (C) 2014-2016 The CyanogenMod Project
 * Copyright (C) 2017 The LineageOS Project
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

import android.content.Context;
import android.os.Message;
import android.os.Parcel;
import android.telephony.PhoneNumberUtils;
import android.telephony.SignalStrength;

import com.android.internal.telephony.cdma.CdmaInformationRecords;
import com.android.internal.telephony.cdma.CdmaInformationRecords.CdmaSignalInfoRec;
import com.android.internal.telephony.cdma.SignalToneUtil;
import com.android.internal.telephony.uicc.IccCardApplicationStatus;
import com.android.internal.telephony.uicc.IccCardStatus;
import com.android.internal.telephony.uicc.IccUtils;

import java.util.ArrayList;
import java.util.Collections;

import static com.android.internal.telephony.RILConstants.*;

/**
 * RIL customization for Galaxy S5 (GSM) LTE devices
 *
 * {@hide}
 */
public class KlteRIL extends RIL {

    public KlteRIL(Context context, int networkMode, int cdmaSubscription) {
        super(context, networkMode, cdmaSubscription, null);
    }

    public KlteRIL(Context context, int preferredNetworkType,
            int cdmaSubscription, Integer instanceId) {
        super(context, preferredNetworkType, cdmaSubscription, instanceId);
    }

    private void
    fixNitz(Parcel p) {
        int dataPosition = p.dataPosition();
        String nitz = p.readString();
        long nitzReceiveTime = p.readLong();

        String[] nitzParts = nitz.split(",");
        if (nitzParts.length >= 4) {
            // 0=date, 1=time+zone, 2=dst, 3(+)=garbage that confuses ServiceStateTracker
            nitz = nitzParts[0] + "," + nitzParts[1] + "," + nitzParts[2];
            p.setDataPosition(dataPosition);
            p.writeString(nitz);
            p.writeLong(nitzReceiveTime);
            // The string is shorter now, drop the extra bytes
            p.setDataSize(p.dataPosition());
        }
    }

    @Override
    protected void
    processUnsolicited(Parcel p, int type) {
        Object ret;
        int dataPosition = p.dataPosition();
        int response = p.readInt();

        switch(response) {
            case RIL_UNSOL_NITZ_TIME_RECEIVED:
                fixNitz(p);
                break;
        }

        p.setDataPosition(dataPosition);
        super.processUnsolicited(p, type);
    }

    @Override
    protected void notifyRegistrantsCdmaInfoRec(CdmaInformationRecords infoRec) {
        final int response = RIL_UNSOL_CDMA_INFO_REC;

        if (infoRec.record instanceof CdmaSignalInfoRec) {
            CdmaSignalInfoRec rec = (CdmaSignalInfoRec) infoRec.record;
            if (rec != null
                    && rec.isPresent
                    && rec.signalType == SignalToneUtil.IS95_CONST_IR_SIGNAL_IS54B
                    && rec.alertPitch == SignalToneUtil.IS95_CONST_IR_ALERT_MED
                    && rec.signal == SignalToneUtil.IS95_CONST_IR_SIG_IS54B_L) {
                /* Drop record, otherwise IS95_CONST_IR_SIG_IS54B_L tone will
                 * continue to play after the call is connected */
                return;
            }
        }
        super.notifyRegistrantsCdmaInfoRec(infoRec);
    }
}
