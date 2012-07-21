#include "StdAfx.h"

#include "TextManager.hpp"

//this snippet ensures that the location of memory leaks is reported correctly in debug mode
#if defined(_DEBUG)
#define new new(_NORMAL_BLOCK,__FILE__, __LINE__)
#endif

namespace MGDF { namespace Test {

#define MAX_LINES 100

TextManagerState::TextManagerState(const TextManagerState *state)
{
	for (auto iter=state->_lines.begin();iter!=state->_lines.end();++iter)
	{
		_lines.push_back(*iter);
	}
}

void TextManagerState::AddLine(unsigned int color,const std::string &line)
{
	Line l;
	l.Color = color;
	l.Content = line;

	if (_lines.size()==MAX_LINES)
	{
		_lines.pop_back();
	}
	_lines.insert(_lines.begin(),l);
}

void TextManagerState::SetStatus(unsigned int color,const std::string &text)
{
	_lines[0].StatusText = text;
	_lines[0].StatusColor = color;
}

boost::shared_ptr<TextManagerState> TextManagerState::Interpolate(const TextManagerState *endState,double alpha)
{
	//interpolation isn't really possible with this type of gamestate, so just use the most recent.
	return boost::shared_ptr<TextManagerState>(new TextManagerState(endState));
}

TextManager::~TextManager()
{
	SAFE_RELEASE(_font);
	SAFE_RELEASE(_immediateContext);
}

TextManager::TextManager(ISystem *system)
	: _system(system)
	, _font(nullptr)
{
	system->GetGraphics()->GetD3DDevice()->GetImmediateContext(&_immediateContext);

	IFW1Factory *factory;
	if (!FAILED(FW1CreateFactory(FW1_VERSION,&factory)))
	{
		factory->CreateFontWrapper(system->GetGraphics()->GetD3DDevice(), L"Arial", &_font);
		SAFE_RELEASE(factory);
	}
}

void TextManager::SetState(boost::shared_ptr<TextManagerState> state)
{
	_state = state;
}

void TextManager::DrawText()
{
	if (_state && _font)
	{
		int starty;
		if (_state.get()->_lines.size()*25 < _system->GetGraphics()->GetScreenY())
		{
		   starty = (_state.get()->_lines.size()*25) - 25;
		}
		else {
			starty = _system->GetGraphics()->GetScreenY() - 25;
		}

		for (auto iter = _state.get()->_lines.begin();iter!=_state.get()->_lines.end();++iter)
		{
			std::wstring content;
			content.assign(iter->Content.begin(),iter->Content.end());

			_font->DrawString(
				_immediateContext,
				content.c_str(),// String
				22.0f,// Font size
				0.0f,// X position
				static_cast<float>(starty),// Y position
				iter->Color,// Text color
				0// Flags (for example FW1_RESTORESTATE to keep context states unchanged)
			);

			if (iter->StatusText!="")
			{
				std::wstring statusText;
				statusText.assign(iter->StatusText.begin(),iter->StatusText.end());

				_font->DrawString(
					_immediateContext,
					statusText.c_str(),
					22.0f,// Font size
					static_cast<float>(_system->GetGraphics()->GetScreenX())-200.0f,// X position
					static_cast<float>(starty),// Y position
					iter->StatusColor,// Text color
					0// Flags (for example FW1_RESTORESTATE to keep context states unchanged)
				);
			}

			starty-=25;
			if (starty<=-25) break;
		}
	}
}

}}