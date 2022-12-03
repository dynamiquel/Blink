#pragma once

UENUM(BlueprintType)
enum class EAsclepiusImmunityState : uint8
{
	/**
	 * @brief The Actor can take damage.
	 */
	None,
	/**
	 * @brief The Actor does not take damage under normal circumstances (may still take damage from sources such as falling out of the map)
	 * and this should be visible in some way.
	 *
	 * i.e. an immunity ability where the health bar changes colour to indicate that the Actor is immune.
	 */
	Invulnerable,
	/**
	* @brief The Actor does not take damage in any way and this should not be visible.
	* 
	* i.e. immunity frames and immune during transitions.
	*/
	Immune
};
