#pragma once

UENUM(BlueprintType)
enum class EAsclepiusLifeState : uint8
{
	/**
	 * @brief The Actor is alive.
	 */
	Alive,
	/**
	 * @brief The Actor is downed. Currently, this has no defined behaviour.
	 */
	Downed,
	/**
	* @brief The Actor is dead.
	*/
	Dead
};
