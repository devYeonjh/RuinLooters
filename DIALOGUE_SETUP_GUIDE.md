# AI NPC 대화 시스템 설정 가이드

## 구현 완료된 컴포넌트

### 1. 핵심 시스템 파일
- `Dialogue/RLDialogueTypes.h/cpp` - 핵심 데이터 구조
- `Dialogue/RLLLMServiceInterface.h/cpp` - LLM 서비스 추상 인터페이스
- `Dialogue/RLMockLLMService.h/cpp` - 테스트용 Mock LLM 서비스
- `Dialogue/RLDialogueManager.h/cpp` - 대화 오케스트레이션 컴포넌트
- `Dialogue/RLAIDialogueNPC.h/cpp` - AI 대화 NPC 클래스
- `UI/RLDialogueWidget.h/cpp` - 대화 UI 위젯

## 블루프린트 설정 단계

### 1. 대화 위젯 블루프린트 생성
1. `Content/UI/` 폴더에 새 User Widget 블루프린트 생성
2. 이름: `WBP_DialogueWidget`
3. Parent Class: `RLDialogueWidget`

#### 필수 UI 컴포넌트 (정확한 이름으로 생성):
```
- MessageInputBox (EditableTextBox) - 플레이어 메시지 입력
- SendButton (Button) - 메시지 전송 버튼
- EndConversationButton (Button) - 대화 종료 버튼
- ConversationScrollBox (ScrollBox) - 대화 내용 스크롤
- NPCNameText (TextBlock) - NPC 이름 표시
- StatusText (TextBlock) - 상태 메시지
- ConversationHistory (VerticalBox) - 대화 내역 컨테이너
```

### 2. AI 대화 NPC 블루프린트 생성
1. `Content/NPCs/` 폴더에 새 Blueprint 생성
2. Parent Class: `RLAIDialogueNPC`
3. 이름: `BP_AIDialogueNPC`

#### 컴포넌트 설정:
- **Dialogue Widget Class**: `WBP_DialogueWidget` 선택
- **Personality** 설정:
  - Character Name: 원하는 NPC 이름
  - Personality Prompt: NPC 성격 설명
  - Background Story: 배경 스토리
  - Speaker ID: TTS 음성 ID (비워두면 기본 음성)

### 3. 레벨에 배치 및 테스트
1. `BP_AIDialogueNPC`를 레벨에 배치
2. 플레이어가 NPC에 접근하면 자동으로 대화 시작
3. 대화 UI가 표시되고 텍스트 입력 가능

## 테스트 시나리오

### Mock LLM 서비스 테스트 응답
- "hello", "hi" → 인사 응답
- "how are you" → 안부 응답  
- "what name" → 이름 응답
- "help" → 도움말 응답
- "bye" → 작별 응답
- 기타 입력 → 랜덤 응답

### 시스템 플로우 테스트
1. 플레이어가 NPC 접근 → 인사 메시지 자동 재생
2. 대화 UI 자동 표시
3. 플레이어 메시지 입력 → "Thinking..." 상태
4. 2초 후 NPC 응답 → TTS + A2F 재생
5. 대화 종료 버튼 또는 플레이어 이동으로 종료

## 고급 설정

### 1. 커스텀 LLM 서비스 구현
`RLLLMServiceInterface`를 상속받아 실제 LLM API 구현:
```cpp
UCLASS()
class UMyLLMService : public URLLMServiceInterface
{
    // OpenAI API, Azure AI, 또는 로컬 LLM 구현
};
```

### 2. NPC 성격 DataTable 사용
1. DataTable 생성 (Row Type: `FNPCPersonalityTableRow`)
2. 여러 NPC 성격 정의
3. 블루프린트에서 DataTable 참조

### 3. 실제 TTS 서버 연동
- CosyVoice 서버 실행: `python run_server.py --port 50001`
- TTSManager 컴포넌트가 자동으로 음성 생성 및 A2F 연동

## 트러블슈팅

### 1. 대화 UI가 표시되지 않는 경우
- `DialogueWidgetClass`가 올바르게 설정되었는지 확인
- UI 컴포넌트 이름이 정확한지 확인

### 2. TTS가 작동하지 않는 경우
- CosyVoice 서버 실행 상태 확인
- TTSManager 컴포넌트가 NPC에 있는지 확인

### 3. LLM 응답이 없는 경우
- Mock LLM 서비스가 올바르게 설정되었는지 확인
- 로그에서 에러 메시지 확인

## 로그 모니터링
```
LogTemp: DialogueManager - 대화 관련 로그
LogTemp: MockLLMService - Mock LLM 응답 로그
LogTemp: AIDialogueNPC - NPC 상태 로그
LogTemp: DialogueWidget - UI 이벤트 로그
```

## 확장 가능성
1. **다중 언어 지원** - Personality Prompt 다국어 설정
2. **감정 표현** - A2F 감정 파라미터 연동
3. **퀘스트 연동** - 대화 결과에 따른 게임 이벤트 트리거
4. **음성 인식** - 플레이어 음성 입력 지원
5. **대화 히스토리** - 세이브/로드 시스템 연동