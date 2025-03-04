/*
 * Copyright (C) 2014 The Android Open Source Project
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

//#define LOG_NDEBUG 0
#define LOG_TAG "ClearKeyCryptoPlugin"
#include <utils/Log.h>

#include <media/stagefright/MediaErrors.h>
#include <utils/StrongPointer.h>

#include "DrmPlugin.h"

#include "Session.h"

namespace {
const android::String8 kStreaming("Streaming");
const android::String8 kOffline("Offline");
const android::String8 kTrue("True");

const android::String8 kQueryKeyLicenseType("LicenseType");
    // Value: "Streaming" or "Offline"
const android::String8 kQueryKeyPlayAllowed("PlayAllowed");
    // Value: "True" or "False"
const android::String8 kQueryKeyRenewAllowed("RenewAllowed");
    // Value: "True" or "False"
};

namespace clearkeydrm {

using android::sp;

DrmPlugin::DrmPlugin(SessionLibrary* sessionLibrary)
        : mSessionLibrary(sessionLibrary) {
    mPlayPolicy.clear();
}

status_t DrmPlugin::openSession(Vector<uint8_t>& sessionId) {
    sp<Session> session = mSessionLibrary->createSession();
    sessionId = session->sessionId();
    return android::OK;
}

status_t DrmPlugin::closeSession(const Vector<uint8_t>& sessionId) {
    sp<Session> session = mSessionLibrary->findSession(sessionId);
    if (sessionId.size() == 0) {
        return android::BAD_VALUE;
    }
    if (session.get()) {
        mSessionLibrary->destroySession(session);
        return android::OK;
    }
    return android::ERROR_DRM_SESSION_NOT_OPENED;
}

status_t DrmPlugin::getKeyRequest(
        const Vector<uint8_t>& scope,
        const Vector<uint8_t>& initData,
        const String8& mimeType,
        KeyType keyType,
        const KeyedVector<String8, String8>& optionalParameters,
        Vector<uint8_t>& request,
        String8& defaultUrl,
        DrmPlugin::KeyRequestType *keyRequestType) {
    UNUSED(optionalParameters);
    if (scope.size() == 0) {
        return android::BAD_VALUE;
    }

    if (keyType != kKeyType_Streaming) {
        return android::ERROR_DRM_CANNOT_HANDLE;
    }

    *keyRequestType = DrmPlugin::kKeyRequestType_Initial;
    defaultUrl.clear();
    sp<Session> session = mSessionLibrary->findSession(scope);
    if (!session.get()) {
        return android::ERROR_DRM_SESSION_NOT_OPENED;
    }

    return session->getKeyRequest(initData, mimeType, &request);
}

void DrmPlugin::setPlayPolicy() {
    android::Mutex::Autolock lock(mPlayPolicyLock);
    mPlayPolicy.clear();
    mPlayPolicy.add(kQueryKeyLicenseType, kStreaming);
    mPlayPolicy.add(kQueryKeyPlayAllowed, kTrue);
    mPlayPolicy.add(kQueryKeyRenewAllowed, kTrue);
}

status_t DrmPlugin::provideKeyResponse(
        const Vector<uint8_t>& scope,
        const Vector<uint8_t>& response,
        Vector<uint8_t>& keySetId) {
    if (scope.size() == 0 || response.size() == 0) {
        return android::BAD_VALUE;
    }
    sp<Session> session = mSessionLibrary->findSession(scope);
    if (!session.get()) {
        return android::ERROR_DRM_SESSION_NOT_OPENED;
    }

    setPlayPolicy();
    status_t res = session->provideKeyResponse(response);
    if (res == android::OK) {
        // This is for testing AMediaDrm_setOnEventListener only.
        sendEvent(kDrmPluginEventVendorDefined, 0, &scope, NULL);
        keySetId.clear();
    }
    return res;
}

status_t DrmPlugin::getPropertyString(
        const String8& name, String8& value) const {
    if (name == "vendor") {
        value = "Google";
    } else if (name == "version") {
        value = "1.0";
    } else if (name == "description") {
        value = "ClearKey CDM";
    } else if (name == "algorithms") {
        value = "";
    } else if (name == "listenerTestSupport") {
        value = "true";
    } else {
        ALOGE("App requested unknown string property %s", name.string());
        return android::ERROR_DRM_CANNOT_HANDLE;
    }
    return android::OK;
}

status_t DrmPlugin::queryKeyStatus(
        const Vector<uint8_t>& sessionId,
        KeyedVector<String8, String8>& infoMap) const {

    if (sessionId.size() == 0) {
        return android::BAD_VALUE;
    }

    infoMap.clear();
    for (size_t i = 0; i < mPlayPolicy.size(); ++i) {
        infoMap.add(mPlayPolicy.keyAt(i), mPlayPolicy.valueAt(i));
    }
    return android::OK;
}
}  // namespace clearkeydrm
