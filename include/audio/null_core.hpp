#pragma once
#include <array>
#include "audio/dsp_core.hpp"

namespace Audio {
	class NullDSP : public DSPCore {
		enum class DSPState : u32 {
			Off,
			On,
			Slep,
		};

		// Number of DSP pipes
		static constexpr size_t pipeCount = 8;
		DSPState dspState;

		std::array<std::vector<u8>, pipeCount> pipeData;  // The data of each pipe
		std::array<u8, Memory::DSP_RAM_SIZE> dspRam;

		void resetAudioPipe();

	  public:
		NullDSP(Memory& mem, Scheduler& scheduler, DSPService& dspService) : DSPCore(mem, scheduler, dspService) {}

		void reset() override;
		void runAudioFrame() override {}
		u8* getDspMemory() override { return dspRam.data(); }

		u16 recvData(u32 regId) override;
		bool recvDataIsReady(u32 regId) override { return true; } // Treat data as always ready
		void writeProcessPipe(u32 channel, u32 size, u32 buffer) override;
		std::vector<u8> readPipe(u32 channel, u32 peer, u32 size, u32 buffer) override;

		// NOPs for null DSP core
		void loadComponent(std::vector<u8>& data, u32 programMask, u32 dataMask) override {}
		void unloadComponent() override {}
		void setSemaphore(u16 value) override {}
		void setSemaphoreMask(u16 value) override {}
	};

}  // namespace Audio