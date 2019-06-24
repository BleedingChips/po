#pragma once
#include "DirectXTex.h"
#include "../../po/include/po_dx11/dx11_form.h"
using namespace PO;
using namespace PO::Dx11;

inline bool SaveToDDS(device_ptr& dfd, context_ptr& cp, const tex2& t, const std::u16string& name, DXGI_FORMAT DF = DXGI_FORMAT_UNKNOWN)
{
	DirectX::ScratchImage SI;
	if (!SUCCEEDED(DirectX::CaptureTexture(dfd, cp, t.ptr, SI))) return false;
	if (DF != DXGI_FORMAT_UNKNOWN)
		if (!SI.OverrideFormat(DF)) return false;
	if (!SUCCEEDED(DirectX::SaveToDDSFile(SI.GetImages(), SI.GetImageCount(), SI.GetMetadata(), 0, reinterpret_cast<const wchar_t*>(name.c_str())))) return false;
	return true;
}

inline bool SaveToDDS(device_ptr& dfd, context_ptr& cp, const tex3& t, const std::u16string& name, DXGI_FORMAT DF = DXGI_FORMAT_UNKNOWN)
{
	DirectX::ScratchImage SI;
	if (!SUCCEEDED(DirectX::CaptureTexture(dfd, cp, t.ptr, SI))) return false;
	if (DF != DXGI_FORMAT_UNKNOWN)
		if (!SI.OverrideFormat(DF)) return false;
	if (!SUCCEEDED(DirectX::SaveToDDSFile(SI.GetImages(), SI.GetImageCount(), SI.GetMetadata(), 0, reinterpret_cast<const wchar_t*>(name.c_str())))) return false;
	return true;
}

inline bool SaveToTGA(device_ptr& dfd, context_ptr& cp, const tex2& t, const std::u16string& name, DXGI_FORMAT DF = DXGI_FORMAT_UNKNOWN)
{
	DirectX::ScratchImage SI;
	if (!SUCCEEDED(DirectX::CaptureTexture(dfd, cp, t.ptr, SI))) return false;
	if (DF != DXGI_FORMAT_UNKNOWN)
		if (!SI.OverrideFormat(DF)) return false;
	if (!SUCCEEDED(DirectX::SaveToTGAFile(*SI.GetImages(), reinterpret_cast<const wchar_t*>(name.c_str())))) return false;
	return true;
}

inline bool LoadFormDDS(creator& c, tex3& Texture, const std::u16string& Name)
{
	DirectX::ScratchImage SI;
	if (!SUCCEEDED(DirectX::LoadFromDDSFile(reinterpret_cast<const wchar_t*>(Name.c_str()), 0, nullptr, SI))) return false;
	DirectX::TexMetadata MetaData = SI.GetMetadata();
	tex3_source tem{ SI.GetPixels(), static_cast<uint32_t>(SI.GetImages()->rowPitch), static_cast<uint32_t>(SI.GetImages()->slicePitch) };
	if (!Texture.create(c, MetaData.format,
	{ static_cast<uint32_t>(MetaData.width), static_cast<uint32_t>(MetaData.height), static_cast<uint32_t>(MetaData.depth) },
		1, false, &tem)) return false;
	return true;
}

inline bool LoadFormDDS(creator& c, tex2& Texture, const std::u16string& Name)
{
	DirectX::ScratchImage SI;
	if (!SUCCEEDED(DirectX::LoadFromDDSFile(reinterpret_cast<const wchar_t*>(Name.c_str()), 0, nullptr, SI))) return false;
	DirectX::TexMetadata MetaData = SI.GetMetadata();
	tex2_source tem{ SI.GetPixels(), static_cast<uint32_t>(SI.GetImages()->rowPitch) };
	if (!Texture.create(c, MetaData.format,
	{ static_cast<uint32_t>(MetaData.width), static_cast<uint32_t>(MetaData.height) },
		1, false, &tem)) return false;
	return true;
}

inline bool LoadFormTGA(creator& c, tex2& Texture, const std::u16string& Name)
{
	DirectX::ScratchImage SI;
	if (!SUCCEEDED(DirectX::LoadFromTGAFile(reinterpret_cast<const wchar_t*>(Name.c_str()), nullptr, SI))) return false;
	DirectX::TexMetadata MetaData = SI.GetMetadata();
	tex2_source tem{ SI.GetPixels(), static_cast<uint32_t>(SI.GetImages()->rowPitch) };
	if (!Texture.create(c, MetaData.format,
	{ static_cast<uint32_t>(MetaData.width), static_cast<uint32_t>(MetaData.height) },
		1, false, &tem)) return false;
	return true;
}