#pragma once


namespace Server {
	class Section;
	class SectionRepository {
	public:
		static System::Future<std::shared_ptr<Section>> BeginEnter(int32_t map_uid);
	};

}

