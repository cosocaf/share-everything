/**
 * @file pch.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief
 * @version 0.1
 * @date 2022-08-26
 *
 * プリコンパイル済みヘッダ。他全てのヘッダでインクルードする。
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef SHARE_EVERYTHING_PCH_H_
#define SHARE_EVERYTHING_PCH_H_

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#define STRICT

#include <Windows.h>
#include <objidl.h>
#include <tchar.h>

#include <algorithm>

namespace Gdiplus {
  using std::max;
  using std::min;
} // namespace Gdiplus

#include <gdiplus.h>

#endif // SHARE_EVERYTHING_PCH_H_