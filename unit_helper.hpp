#pragma once
namespace content {	namespace unit {

	struct UUId
	{
		uint8_t _serverNo;
		uint8_t _serverType;
		uint16_t _seq;
		uint32_t _time;

		UUId()
		{
		}

		UUId(uint64_t id_)
		{
			_serverNo = (uint8_t)(id_ >> 56);
			_serverType = (uint8_t)((0x00FF000000000000 & id_) >> 48);
			_seq = (uint16_t)((0x0000FFFF00000000 & id_) >> 32);
			_time = (0x00000000FFFFFFFF & id_);
		}

		operator uint64_t() {
			return ((uint64_t)(_serverNo) << 56 ) + ((uint64_t)(_serverType) << 48) + ((uint64_t)(_seq) << 32) + (uint64_t)(_time);
		}
	};


	static uint64_t genUUId(uint8_t serverNo_, uint8_t serverType_, uint16_t seq_, uint64_t time_)
	{
		UUId id;
		id._serverNo = serverNo_;
		id._serverType = serverType_;
		id._seq = seq_;
		id._time = (uint32_t)(time_);
		return id;
	}

	static uint8_t getServerNo(uint64_t id_)
	{
		return UUId(id_)._serverNo;
	}

	static uint8_t getServerType(uint64_t id_)
	{
		return UUId(id_)._serverType;
	}

	static uint16_t getSeq(uint64_t id_)
	{
		return UUId(id_)._seq;
	}

	static uint64_t getTime(uint64_t id_)
	{
		return UUId(id_)._time;
	}

	class vector2
	{
	public:
		vector2() {}
		vector2(float x_, float y_): x(x_), y(y_) {}

		vector2& operator +=(const vector2& v_) { x += v_.x; y += v_.y; return *this; }
		vector2& operator -=(const vector2& v_) { x -= v_.x; y -= v_.y; return *this; }
		vector2& operator *=(float v_) { x *= v_; y *= v_; return *this; }
		vector2& operator /=(float v_) { x /= v_; y /= v_; return *this; }
		vector2 operator -() { return vector2{ -x, -y }; }
		vector2& operator =(const vector2& v_) { x = v_.x; y = v_.y; return *this; }
		bool operator ==(const vector2& v_) { /*x = v_.x; y = v_.y;*/ return false; }

	public:
		float x = 0.f;
		float y = 0.f;

	};

	enum class Species : uint32_t
	{
		SPECIES_NONE = 0,
		SPECIES_BG_OBJECT = 1,
		SPECIES_SIMPLE_OBJECT = 2,
		SPECIES_MAX
	};
}}

