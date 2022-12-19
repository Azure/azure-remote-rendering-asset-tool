#include <mfapi.h>
#include <mfidl.h>
#include <string>
#include <vector>
#include <wrl/client.h>

namespace
{
    template <class T>
    void SafeRelease(T** ppT)
    {
        if (*ppT)
        {
            (*ppT)->Release();
            *ppT = nullptr;
        }
    }

    enum class CodecType
    {
        H264,
        HEVC
    };

    bool GetTransforms(CodecType codec, IMFActivate*** transforms, UINT32& transformsCount)
    {
        MFT_REGISTER_TYPE_INFO formatDec = {MFMediaType_Video, MFVideoFormat_NV12};
        MFT_REGISTER_TYPE_INFO formatEnc = {MFMediaType_Video, MFVideoFormat_Base};

        switch (codec)
        {
            case CodecType::H264:
                formatEnc.guidSubtype = MFVideoFormat_H264;
                break;

            case CodecType::HEVC:
                formatEnc.guidSubtype = MFVideoFormat_HEVC;
                break;

            default:
                return false;
        };

        // we need video decoding functionality
        GUID categoryGuid = MFT_CATEGORY_VIDEO_DECODER;
        const MFT_REGISTER_TYPE_INFO* formatSrc = &formatEnc;
        const MFT_REGISTER_TYPE_INFO* formatDst = &formatDec;

        HRESULT res;
        if (FAILED(res = MFTEnumEx(categoryGuid, MFT_ENUM_FLAG_ALL | MFT_ENUM_FLAG_SORTANDFILTER, formatSrc, formatDst, transforms, &transformsCount)))
        {
            return false;
        }

        return true;
    }

    struct MftInfo
    {
        CodecType codec;
        HRESULT activationResult = 0;
    };

    uint32_t EnumerateMfts(CodecType codec, std::vector<MftInfo>& mfts)
    {
        IMFActivate** transforms = nullptr;
        UINT32 transformsCount = 0;

        if (!GetTransforms(codec, &transforms, transformsCount))
        {
            return 0;
        }

        if (transforms == nullptr)
        {
            return 0;
        }

        for (uint32_t i = transformsCount; i-- > 0;)
        {
            MftInfo info;
            info.codec = codec;

            Microsoft::WRL::ComPtr<IMFTransform> transform;
            info.activationResult = transforms[i]->ActivateObject(IID_PPV_ARGS(&transform));

            if (SUCCEEDED(info.activationResult))
            {
                transform = nullptr;
                transforms[i]->ShutdownObject();
            }

            mfts.push_back(info);
            SafeRelease(transforms + i);
        }

        CoTaskMemFree(transforms);
        return transformsCount;
    }
} // namespace

bool IsVideoDecoderAvailable(std::string& details)
{
    static bool bCheckedAlready = false;
    if (bCheckedAlready)
        return true;

    HRESULT hr = ::MFStartup(MF_VERSION, MFSTARTUP_FULL);

    if (FAILED(hr))
    {
        details = "Media Foundation Startup failed.";
        return false;
    }

    std::vector<MftInfo> mfts;

    const uint32_t h264Count = EnumerateMfts(CodecType::H264, mfts);
    const uint32_t HEVCCount = EnumerateMfts(CodecType::HEVC, mfts);

    ::MFShutdown();

    if (h264Count == 0 && HEVCCount == 0)
    {
        details = "Neither an H264 nor an HVEC (H265) codec is available.";
        return false;
    }

    // is there any codec that we could activate successfully? -> good enough
    for (const MftInfo& info : mfts)
    {
        if (info.activationResult == 0)
        {
            bCheckedAlready = true;
            return true;
        }
    }

    // no functioning codec available -> give a little bit more detailed error message

    for (const MftInfo& info : mfts)
    {
        switch (info.codec)
        {
            case CodecType::H264:
                details = "H264 codec could not be activated.";
                break;

            case CodecType::HEVC:
                details = "HEVC (H265) codec could not be activated.";
                break;
        }
    }

    return false;
}
