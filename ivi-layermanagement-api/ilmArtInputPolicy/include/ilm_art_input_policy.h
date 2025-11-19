/***************************************************************************
*
* Copyright 2015, Codethink Ltd.
*
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*        http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
****************************************************************************/
#ifndef _ILM_ART_INPUT_POLICY_H_
#define _ILM_ART_INPUT_POLICY_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "ilm_common.h"

struct ilmArtInputPolicyRegion
{
    t_ilm_uint x;       /*!< x coordinate of the top-left corner of the rectangle */
    t_ilm_uint y;       /*!< y coordinate of the top-left corner of the rectangle */
    t_ilm_uint width;   /*!< width of the rectangle */
    t_ilm_uint height;  /*!< height of the rectangle */
};



/**
 * \brief      Set the surface's accepted seats to the list specified
 * \ingroup    ilmArtInputPolicy
 * \param[in]  surfaceID   The target surface ID
 * \param[in]  x           The x coordinate of the top-left corner of the
 *                         rectangle to add
 * \param[in]  y           The y coordinate of the top-left corner of the
 *                         rectangle to add
 * \param[in]  width       The width of the rectangle to add
 * \param[in]  height      The height of the rectangle to add
 * 
 * \return     ILM_SUCCESS if the method call was successful
 * \return     ILM_FAILED  if the client cannot call the method on the surface
 */
ilmErrorTypes
ilm_art_addInputRectangle(t_ilm_surface surfaceID,
                         t_ilm_int x,
                         t_ilm_int y,
                         t_ilm_uint width,
                         t_ilm_uint height);

/**
 * \brief      Clear all input regions from the specified surface
 * \ingroup    ilmArtInputPolicy
 * \param[in]  surfaceID   The target surface ID
 * 
 * \return     ILM_SUCCESS if the method call was successful
 * \return     ILM_FAILED  if the client cannot call the method on the surface
 */
ilmErrorTypes
ilm_art_clearInputRegions(t_ilm_surface surfaceID);

/**
 * \brief      Reset the input region of the specified surface to the full
 *             surface area
 * \ingroup    ilmArtInputPolicy
 * \param[in]  surfaceID   The target surface ID
 * 
 * \return     ILM_SUCCESS if the method call was successful
 * \return     ILM_FAILED  if the client cannot call the method on the surface
 */
ilmErrorTypes
ilm_art_resetInputRegions(t_ilm_surface surfaceID);

/**
 * \brief      Commit the input region changes for the specified surface
 * \ingroup    ilmArtInputPolicy
 * \param[in]  surfaceID   The target surface ID
 * 
 * \return     ILM_SUCCESS if the method call was successful
 * \return     ILM_FAILED  if the client cannot call the method on the surface
 */
ilmErrorTypes
ilm_art_commitInputRegion(t_ilm_surface surfaceID);

#ifdef __cplusplus
} /**/
#endif /* __cplusplus */

#endif /* _ILM_ART_INPUT_POLICY_H_ */
