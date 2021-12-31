#pragma once

struct vec2_t {
	vec2_t() = default;

	float x = {};
	float y = {};

	vec2_t(float i_x, float i_y) {
		x = i_x;
		y = i_x;
	}

	__forceinline vec2_t operator+(const vec2_t& value) const { 
		return vec2_t(x + value.x, y + value.y); 
	}

	__forceinline vec2_t operator-(const vec2_t& value) const { 
		return vec2_t(x - value.x, y - value.y); 
	}

	__forceinline vec2_t operator+(float value) const { 
		return vec2_t(x + value, y + value); 
	}

	__forceinline vec2_t operator-(float value) const { 
		return vec2_t(x - value, y - value); 
	}

	__forceinline vec2_t operator/(float value) const { 
		return vec2_t(x / value, y / value); 
	}

	__forceinline vec2_t operator*(float value) const { 
		return vec2_t(x * value, y * value); 
	}

	__forceinline vec2_t operator-() const { 
		return vec2_t(-x, -y); 
	}

	__forceinline vec2_t& operator-=(const vec2_t& value) {
		x -= value.x;
		y -= value.y;

		return *this;
	}

	__forceinline vec2_t& operator+=(const vec2_t& value) {
		x += value.x;
		y += value.y;

		return *this;
	}

	__forceinline vec2_t& operator/=(float value) {
		x /= value;
		y /= value;

		return *this;
	}

	__forceinline vec2_t& operator*=(float value) {
		x *= value;
		y *= value;

		return *this;
	}

	__forceinline bool operator==(const vec2_t& value) const { 
		return x == value.x && y == value.y; 
	}

	__forceinline bool operator!=(const vec2_t& value) const { 
		return !(operator==(value)); 
	}

	__forceinline bool empty() const { 
		return x == 0.f && y == 0.f; 
	}
};

struct vec3_t {
	vec3_t() = default;

	float x = {};
	float y = {};
	float z = {};

	vec3_t(float i_x, float i_y, float i_z) {
		x = i_x;
		y = i_x;
		z = i_z;
	}

	__forceinline vec3_t operator+(const vec3_t& value) const { 
		return vec3_t(x + value.x, y + value.y, z + value.z); 
	}

	__forceinline vec3_t operator-(const vec3_t& value) const { 
		return vec3_t(x - value.x, y - value.y, z - value.z); 
	}

	__forceinline vec3_t operator-(float value) const { 
		return vec3_t(x - value, y - value, z - value); 
	}

	__forceinline vec3_t operator+(float value) const { 
		return vec3_t(x + value, y + value, z + value); 
	}

	__forceinline vec3_t operator/(float value) const { 
		return vec3_t(x / value, y / value, z / value); 
	}

	__forceinline vec3_t operator*(float value) const { 
		return vec3_t(x * value, y * value, z * value); 
	}

	__forceinline vec3_t operator-() const { return vec3_t(-x, -y, -z); }

	__forceinline vec3_t& operator-=(const vec3_t& value) {
		x -= value.x;
		y -= value.y;
		z -= value.z;

		return *this;
	}

	__forceinline vec3_t& operator+=(const vec3_t& value) {
		x += value.x;
		y += value.y;
		z += value.z;

		return *this;
	}

	__forceinline vec3_t& operator/=(const vec3_t& value) {
		x /= value.x;
		y /= value.y;
		z /= value.z;

		return *this;
	}

	__forceinline vec3_t& operator*=(const vec3_t& value) {
		x *= value.x;
		y *= value.y;
		z *= value.z;

		return *this;
	}

	__forceinline vec3_t& operator/=(float value) {
		x /= value;
		y /= value;
		z /= value;

		return *this;
	}

	__forceinline vec3_t& operator*=(float value) {
		x *= value;
		y *= value;
		z *= value;

		return *this;
	}

	__forceinline bool operator==(const vec3_t& value) const { 
		return x == value.x && y == value.y && z == value.z; 
	}

	__forceinline bool operator!=(const vec3_t& value) const { 
		return !(operator==(value)); 
	}

	__forceinline bool empty() const { 
		return x == 0.f && y == 0.f && z == 0.f; 
	}
};

